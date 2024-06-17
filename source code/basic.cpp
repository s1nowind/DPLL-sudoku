#ifndef BASIC_CPP_
#define BASIC_CPP_
#include <QString>
#include <QMessageBox>

#include "basic.h"

//基础
status AddVar(Clause *p,int target)//头插法 仅用于创建
{
    Variable *vp = (Variable *)malloc(sizeof(Variable));
    if(vp==nullptr)
        return ERROR;
    vp->var = target;
    vp->next = p->first;
    p->first = vp;
    p->length++;
    return TRUE;
}


Variable *RemoveVar(Clause *p, int target)
{
    Variable *vpre=nullptr, *vp;
    for (vp = p->first;vp!=nullptr;vp=vp->next)
    {
        if(vp->var==target)
        {
            p->length--;
            if(vpre==nullptr)
                p->first = p->first->next;
            else
                vpre->next = vp->next;
            vp->next = p->rmfirst;
            p->rmfirst = vp;
            return vp;
        }
        vpre = vp;
    }
    return nullptr;
}

status RecoverVar(Clause *p, Variable *target)//将文字恢复
{
    Variable *vpre = nullptr;
    if(p->rmfirst==target)
        p->rmfirst = target->next;
    else
    {
        for (vpre = p->rmfirst;vpre!=nullptr&&vpre->next!=target;vpre=vpre->next)
            ;
        if(vpre==nullptr)
            return ERROR;
        vpre->next = target->next;
    }
    target->next = p->first;
    p->first = target;
    p->length++;
    return TRUE;
}

//文字索引

status AddClauseIndex(Solver *head, Clause *p)//增加子句文字索引
{
    if(head->var_index==nullptr)
        return FALSE;
    Variable *vp;
    VarIndex *va;
    for (vp = p->first; vp != nullptr;vp=vp->next)
    {
        va = (VarIndex *)malloc(sizeof(VarIndex));
        va->var = vp;
        va->varClause = p;
        va->next = head->var_index[ABS(vp->var)].next;//头插法
        head->var_index[ABS(vp->var)].next = va;
    }
    return TRUE;
}
status DestroyClauseIndex(Solver *head, Clause *p)//删除子句的文字索引
{
    VarIndex *va, *temp;
    for (Variable *vp = p->first; vp != nullptr;vp=vp->next)//未删除的
    {
        for (va = &(head->var_index[ABS(vp->var)]); va->next->varClause != p;va=va->next)
            ;
        temp = va->next;
        va->next = temp->next;
        free(temp);
    }
    for (Variable *vp = p->rmfirst; vp != nullptr;vp=vp->next)//已删除的
    {
        for(va = &(head->var_index[ABS(vp->var)]); va->next->varClause != p;va=va->next)
            ;
        temp = va->next;
        va->next = temp->next;
        free(temp);
    }
    return TRUE;
}

status CreateVarIndex(Solver *head)//建立索引
{
    if(head->var_index!=nullptr||head->var_num==0)
        return FALSE;
    head->var_index = (VarIndex *)malloc(sizeof(VarIndex) * (head->var_num + 1));
    for (int i = 0; i <= head->var_num;++i)
        head->var_index[i].next = nullptr;
    for (Clause *p = head->root; p != nullptr;p=p->next)
        AddClauseIndex(head, p);
    return TRUE;
}

status DestroyAllIndex(Solver *head)//清除所有文字索引
{
    if(head->var_index==nullptr)
        return TRUE;
    for (int i = 1; i <= head->var_num;++i)
    {
        while(head->var_index[i].next!=nullptr)
        {
            VarIndex *va = head->var_index[i].next;
            head->var_index[i].next = va->next;
            free(va);
        }
    }
    return TRUE;
}



//子句处理函数
status AddClause(Solver *head, int *clause, int length)//头插法
{
    Clause *p = (Clause *)malloc(sizeof(Clause));
    if(p==nullptr||length<=0)
        return ERROR;
    p->length = 0;
    p->first = nullptr;
    p->rmfirst = nullptr;
    p->is_delete = FALSE;
    for (int i = 0; i < length;++i)
    {
        if(AddVar(p,*(clause+i))==ERROR)
            return ERROR;
        head->count[CountPos(head, *(clause + i))]++;
    }
    if(head->root!=nullptr)
        head->root->pre = p;
    p->next = head->root;
    p->pre = nullptr;
    head->root = p;
    head->clause_num++;
    return TRUE;
}


status InsertClause(Solver *head, Clause *target)
{
    if(head->root!=nullptr)
        head->root->pre = target;
    target->pre = nullptr;
    target->next = head->root;
    head->root = target;
    target->is_delete = FALSE;
    head->clause_num++;
    return TRUE;
}

Clause *RemoveClause(Solver *head, Clause *p)//只是去除链表中关系
{
    if(p->next!=nullptr)
        p->next->pre = p->pre;
    if(p->pre!=nullptr)
        p->pre->next = p->next;
    else
        head->root = p->next;
    head->clause_num--;
    p->is_delete = TRUE;
    p->pre = nullptr;
    p->next = nullptr;
    return p;
}

status DestroyClause(Solver *head, Clause *p)
{
    Variable *vp;
    if(p->next!=nullptr)
        p->next->pre = p->pre;

    if(p->pre!=nullptr)
        p->pre->next = p->next;
    else
        head->root = p->next;
    for (vp = p->first; vp != nullptr;)
    {
        Variable *temp = vp->next;
        free(vp);
        vp = temp;
    }
    for (vp = p->rmfirst; vp != nullptr;)
    {
        Variable *temp = vp->next;
        free(vp);
        vp = temp;
    }
    if(p->is_delete==FALSE)
        head->clause_num--;
    free(p);
    return TRUE;
}

status DestroyAllClause(Solver *head)
{
    Clause *p = head->root;
    for (p = head->root;p!=nullptr;)
    {
        Clause *temp = p->next;
        DestroyClause(head, p);
        p = temp;
    }
    head->root = nullptr;
    return TRUE;
}

Clause *GetUnitClause(Solver *head)
{
    Clause *p = nullptr;
    for (p = head->root; p != nullptr; p = p->next)
        if (p->length==1)
            return p;
    return p;
}

//求解SAT
int CountPos(Solver *head,int var)
{
    if(var>0)
        return var;
    else
        return -var + head->var_num;
}

status CreateSolver(Solver *head, int var_num, int clause_num)//创建solver内容
{
    head->clause_num=clause_num;
    head->var_num = var_num;
    head->choose_count=0;
    head->root = nullptr;

    head->trailOrder = (int *)malloc(sizeof(int) * (var_num + 1));
    head->trail = (Trail *)malloc(sizeof(Trail) * (var_num + 1));

    head->count = (int *)malloc(sizeof(int) * (var_num * 2 + 1));
    head->var_index = nullptr;
    if(head->count!=nullptr&&head->trailOrder!=nullptr&&head->trail!=nullptr)
    {
        memset(head->trailOrder, 0, sizeof(int) * (head->var_num + 1));
        for (int i = 1; i <= head->var_num;i++)
        {
            head->trail[i].level = -1;
            head->trail[i].tag = -1;
            head->trail[i].var = UNKNOWN;
        }
        memset(head->count, 0, sizeof(int) * (head->var_num * 2 + 1));
        return TRUE;
    }
    return FALSE;
}

status DestroySolver(Solver *head)
{
    if(head==nullptr)
        return FALSE;
    if(head->trailOrder!=nullptr)
        free(head->trailOrder);
    if(head->trail!=nullptr)
        free(head->trail);
    DestroyAllClause(head);
    if(head->count!=nullptr)
        free(head->count);
    DestroyAllIndex(head);
    free(head->var_index);
    free(head);
    return TRUE;
}


//cnfparser
Solver *read_cnffile(char filepath[])
{
    FILE *fp = fopen(filepath, "r");
    Solver *head = (Solver *)malloc(sizeof(Solver));
    if(fp==nullptr||head==nullptr)
        return nullptr;
    int var_num, clause_num;
    char str[MAX_LEN], c;
    while((c=fgetc(fp))!='p')
        fgets(str, MAX_LEN, fp);
    fscanf(fp, "%s%d%d", str, &var_num, &clause_num);
    if(CreateSolver(head,var_num,0)!=TRUE)
    {
        fclose(fp);
        DestroySolver(head);
        return nullptr;
    }
    for (int i = 0; i < clause_num;++i)
    {
        int num[MAX_VAR_NUM], count = 0;
        do{
            fscanf(fp, "%d", &num[count++]);
        } while (num[count - 1] != 0);
        if(AddClause(head,num,count-1)!=TRUE)
        {
            fclose(fp);
            return nullptr;
        }
    }
    fclose(fp);
    CreateVarIndex(head);
    return head;
}
status save_resfile(char filepath[], status state, Solver *head, double time)
{
    char suffix[4] = {"res"}, file[1000];
    strcpy(file, filepath);
    int i = 0;
    while(file[i++]!='\0')
        ;
    i-=2;
    for (int j = 1;j<=3;++j)
        file[i - j + 1] = suffix[3 - j];
    FILE *fp = fopen(file, "w");
    if(fp==nullptr)
        return ERROR;
    fprintf(fp, "s %d\n", state);
    if(state==TRUE)
    {
        fprintf(fp,"v ");
        for(int i=1;i<=head->var_num;++i)
            fprintf(fp," %d",head->trail[i].var);
        fprintf(fp,"\n");
    }
    fprintf(fp, "t %fms\n", time);
    fclose(fp);
    return TRUE;
}
status save_cnffile(char filepath[], Solver *head)
{
    FILE *fp = fopen(filepath, "w");
    if(fp==nullptr)
        return ERROR;
    fprintf(fp, "p cnf %d %d\n", head->var_num, head->clause_num);
    for (Clause *p = head->root; p != nullptr;p=p->next)
    {
        if(p->is_delete==TRUE)
        {
            printf("error!\n");
            system("pause");
        }
        for (Variable *vp = p->first; vp != nullptr;vp=vp->next)
            fprintf(fp, "%d ", vp->var);
        fprintf(fp, "0\n");
    }
    fclose(fp);
    return TRUE;
}

//DPLLsolver

//策略
int MOM_strategy(Solver *head)
{
    double *num = (double *)malloc(sizeof(double) * (head->var_num + 1));
    memset(num, 0, sizeof(double) * (head->var_num + 1));
    for (Clause *p = head->root; p != nullptr;p=p->next)
    {
        for (Variable *vp = p->first; vp != nullptr;vp=vp->next)
            num[ABS(vp->var)] += 1.0 / (1 << p->length);
    }
    double max = -1.0;
    int index = 0;
    for (int i = 1; i <= head->var_num;++i)
    {
        if(max<num[i]&&head->trail[i].var==UNKNOWN)
        {
            max = num[i];
            index = i;
        }
    }
    int pos = 0, neg = 0;
    VarIndex *va = head->var_index[index].next;
    while(va!=nullptr)
    {
        if(va->var->var>0)
            pos++;
        else
            neg++;
        va = va->next;
    }
    free(num);
    return pos >= neg ? index : -index;
}

int VSIDS_strategy(Solver *head)
{
    int max = -1, index = 0;
    for (int i = 1; i <= head->var_num;++i)
    {
        if(head->trail[i].var==UNKNOWN&&max<(head->count[i]+head->count[i+head->var_num]))
        {
            index = i;
            max = head->count[i] + head->count[i + head->var_num];
        }
    }
    head->choose_count++;
    if(head->choose_count>=VSIDSCOUNT)
    {
        head->choose_count = 0;
        for (int i = 1; i < head->var_num * 2;++i)
            head->count[i] /= 3;
    }
    return head->count[index] >= head->count[index + head->var_num] ? index : -index;
}

int First_strategy(Solver *head)
{
    for (int i = 1; i <= head->var_num;++i)
        if(head->trail[i].var==UNKNOWN)
            return -i;
    return 0;
}

int Choose_strategy(Solver *head, enum strategy s)
{
    int index=0;
        switch(s)
        {
            case mom:
                index = MOM_strategy(head);
                break;
            case vsids:
                index = VSIDS_strategy(head);
                break;
            case first:
                index = First_strategy(head);
                break;
            default:
                break;
        }
        return index;
}

status BCP(Solver *head, int level, int var, int tag, ChangeStack *sk)
{
    if(tag)
        SaveTrail(head, NOTDEC, var, level);
    VarIndex *va = head->var_index[ABS(var)].next;
    while(va!=nullptr)
    {
        if(va->varClause->is_delete==FALSE)
        {
            if(va->var->var==var)
            {
                RemoveClause(head, va->varClause);
                SaveChange(va->varClause, nullptr, CLAUSE, sk, level);
            }
            else if(va->var->var==-var)
            {
                Variable *temp = RemoveVar(va->varClause, va->var->var);
                if(temp==nullptr)
                    system("pause");
                SaveChange(va->varClause, va->var, VARIABLE, sk, level);
                if(va->varClause->length==1)
                {
                    //RemoveClause(head, va->varClause);
                    //InsertClause(head, va->varClause);
                    Clause *p = va->varClause;
                    if(p->pre!=nullptr)
                    {
                        p->pre->next = p->next;
                        if(p->next!=nullptr)
                            p->next->pre = p->pre;
                        p->next = head->root;
                        head->root->pre = p;
                        head->root = p;
                        p->pre = nullptr;
                    }
                }
                else if(va->varClause->length==0)
                return FALSE;
            }
        }
        va = va->next;
    }
    return TRUE;
}
status SaveChange(Clause *cla, Variable *vari, int tag, ChangeStack *sk, int level)
{
    ChangeStack *newstack = (ChangeStack *)malloc(sizeof(ChangeStack));
    newstack->tag = tag;
    newstack->level = level;
    newstack->cla = cla;
    newstack->vari = vari;
    newstack->next = sk->next;
    sk->next = newstack;
    return TRUE;
}

status BackJump(Solver *head, int back, ChangeStack *sk)
{
    ChangeStack *s = sk->next;
    while(s!=nullptr&&s->level>back)
    {
        if(s->tag==VARIABLE)
        {
            if(RecoverVar(s->cla, s->vari)==ERROR)
                printf("sadas\n");
        }
        else if(s->tag==CLAUSE)
        {
            InsertClause(head, s->cla);
        }
        sk->next = sk->next->next;
        free(s);
        s = sk->next;
    }
    return TRUE;
}

int BackTrail(Solver *head, int level)//返回决策变量
{
    int order = head->trailOrder[0];
    while(order>0&&head->trail[head->trailOrder[order]].level>level)
        order--;
    int var = head->trail[head->trailOrder[order+1]].var;
    for (int i = head->trailOrder[0]; i > order; --i)
    {
        int pos = head->trailOrder[i];
        head->trail[pos].level = -1;
        head->trail[pos].tag = 0;
        head->trail[pos].var = UNKNOWN;
    }
    head->trailOrder[0] = order;
    return var;
}

status SaveTrail(Solver *head, int tag, int var, int level)//同时保存层级
{
    head->trailOrder[0]++;
    int pos = head->trailOrder[0];
    head->trailOrder[pos] = ABS(var);
    head->trail[ABS(var)].tag = tag;
    head->trail[ABS(var)].level = level;
    head->trail[ABS(var)].var = var;
    return TRUE;
}



boolean DPLL(Solver *head,enum strategy s)//按决策层级来进行回溯
{
    int level = 0, res = TRUE, var = 0; //从零开始
    ChangeStack sk;
    sk.next = nullptr;
    Clause *unit = GetUnitClause(head);
    if(unit!=nullptr)
        var = unit->first->var;
    int tag = 1;
    while(1)
    {
        if(var)
            res = BCP(head, level, var, tag, &sk);
        if(res==TRUE)
        {
            unit = GetUnitClause(head);
            while(unit!=nullptr)
            {
                var = unit->first->var;
                res = BCP(head, level, var, 1, &sk);
                if(res==FALSE)
                    break;
                unit = GetUnitClause(head);
            }
        }
        if(res==FALSE)
        {
            if(level==0)
                return FALSE;
            tag = 0;
            level--;
            BackJump(head, level, &sk);
            var = BackTrail(head, level);
            var = -var;
            SaveTrail(head, DEC, var, level);
        }
        else
        {
            if(head->root==nullptr)
                return TRUE;
            tag = 0;
            level++;
            var = Choose_strategy(head, s);
            SaveTrail(head, DEC, var, level);
        }

    }
    return TRUE;
}

boolean DPLL_sudoku(Solver *head,int *num)//按决策层级来进行回溯
{
    int level = 0, res = TRUE, var = 0; //从零开始
    ChangeStack sk;
    sk.next = nullptr;
    Clause *unit = GetUnitClause(head);
    if(unit!=nullptr)
        var = unit->first->var;
    int tag = 1;
    while(1)
    {
        if(var)
            res = BCP(head, level, var, tag, &sk);
        if(res==TRUE)
        {
            unit = GetUnitClause(head);
            while(unit!=nullptr)
            {
                var = unit->first->var;
                res = BCP(head, level, var, 1, &sk);
                if(res==FALSE)
                    break;
                unit = GetUnitClause(head);
            }
        }
        if(res==FALSE)
        {
            if(level==0)
                return FALSE;
            tag = 0;
            level--;
            BackJump(head, level, &sk);
            var = BackTrail(head, level);
            var = -var;
            SaveTrail(head, DEC, var, level);
        }
        else
        {
            if(head->root==nullptr)
            {
                (*num)++;
                if((*num)>=2) return TRUE;
                if(level==0)
                    return FALSE;
                tag = 0;
                level--;
                BackJump(head, level, &sk);
                var = BackTrail(head, level);
                var = -var;
                SaveTrail(head, DEC, var, level);
                continue;
            }
            tag = 0;
            level++;
            var = Choose_strategy(head, vsids);
            SaveTrail(head, DEC, var, level);
        }

    }
    return TRUE;
}

//sudoku
int pos_to_num(int x, int a, int b, char num)
{
    return (x - 1) * 729 + (a - 1) * 81 + (b - 1) * 9 + (int)num;
}

void sudoku_to_sat(char sudoku1[][10], char sudoku2[][10])
{
    FILE *fp = fopen("temp", "w");
    int clauseNum = 0;
    for (int i = 1;i<=9;++i)//已存在的提示
        for (int j = 1;j<=9;++j)
        {
            if(sudoku1[i][j]!=0)
            {
                fprintf(fp, "%d 0\n", pos_to_num(1, i, j, sudoku1[i][j]));
                clauseNum++;
            }
            if(sudoku2[i][j]!=0)
            {
                fprintf(fp, "%d 0\n", pos_to_num(2, i, j, sudoku2[i][j]));
                clauseNum++;
            }
        }

    for (int i = 7;i<=9;++i)//处理重叠部分
        for (int j = 7;j<=9;++j)
            for (char num = 1;num<=9;++num)
            {
               fprintf(fp, "%d -%d 0\n", pos_to_num(1, i, j, num), pos_to_num(2, i - 6, j - 6, num));
               fprintf(fp, "-%d %d 0\n", pos_to_num(1, i, j, num), pos_to_num(2, i - 6, j - 6, num));
               clauseNum += 2;
            }

    for (int i = 1;i<=9;++i)//处理单独格子
        for (int j = 1;j<=9;++j)
        {
            if(sudoku1[i][j]!=0)//非空
            {
                for (char num = 1;num<=9;++num)
                {
                    if(num!=sudoku1[i][j])
                        fprintf(fp, "-%d 0\n", pos_to_num(1, i, j, num));
                }
                clauseNum += 8;
            }
            else if(sudoku1[i][j]==0)
            {
                for (char num = 1;num<=9;++num)
                    fprintf(fp, "%d ", pos_to_num(1, i, j, num));
                fprintf(fp, "0\n");
                for (char m = 1;m<=8;++m)
                    for (char n = m + 1;n<=9;++n)
                        fprintf(fp, "-%d -%d 0\n", pos_to_num(1, i, j, m), pos_to_num(1, i, j, n));
                clauseNum += 37;
            }
            if(sudoku2[i][j]!=0)//非空
            {
                for (char num = 1;num<=9;++num)
                {
                    if(num!=sudoku2[i][j])
                        fprintf(fp, "-%d 0\n", pos_to_num(2, i, j, num));
                }
                clauseNum += 8;
            }
            else if(sudoku2[i][j]==0)
            {
                for (char num = 1;num<=9;++num)
                    fprintf(fp, "%d ", pos_to_num(2, i, j, num));
                fprintf(fp, "0\n");
                for (char m = 1;m<=8;++m)
                    for (char n = m + 1;n<=9;++n)
                        fprintf(fp, "-%d -%d 0\n", pos_to_num(2, i, j, m), pos_to_num(2, i, j, n));
                clauseNum += 37;
            }
        }

    for (int i = 1;i<=9;++i)//行约束
        for (char num = 1;num<=9;++num)
        {
            for (int j = 1;j<=9;++j)//第i行有num
                fprintf(fp, "%d ", pos_to_num(1, i, j, num));
            fprintf(fp, "0\n");
            clauseNum++;
            for (int m = 1;m<=8;++m)//第m和n个格子不同时为num
                for (int n = m + 1;n<=9;++n)
                    fprintf(fp, "-%d -%d 0\n", pos_to_num(1, i, m, num), pos_to_num(1, i, n, num));
            clauseNum += 36;

            for (int j = 1;j<=9;++j)
                fprintf(fp, "%d ", pos_to_num(2, i, j, num));
            fprintf(fp, "0\n");
            clauseNum++;
            for (int m = 1;m<=8;++m)//第m和n个格子不同时为num
                for (int n = m + 1;n<=9;++n)
                    fprintf(fp, "-%d -%d 0\n", pos_to_num(2, i, m, num), pos_to_num(2, i, n, num));
            clauseNum += 36;
        }

    for (int j = 1;j<=9;++j)//列约束
        for (char num = 1;num<=9;++num)
        {
            for (int i = 1;i<=9;++i)//第j列有num
                fprintf(fp, "%d ", pos_to_num(1, i, j, num));
            fprintf(fp, "0\n");
            clauseNum++;
            for (int m = 1;m<=8;++m)
                for (int n = m + 1;n<=9;++n)
                    fprintf(fp, "-%d -%d 0\n", pos_to_num(1, m, j, num), pos_to_num(1, n, j, num));
            clauseNum += 36;

            for (int i = 1;i<=9;++i)//第j列有num
                fprintf(fp, "%d ", pos_to_num(2, i, j, num));
            fprintf(fp, "0\n");
            clauseNum++;
            for (int m = 1;m<=8;++m)
                for (int n = m + 1;n<=9;++n)
                    fprintf(fp, "-%d -%d 0\n", pos_to_num(2, m, j, num), pos_to_num(2, n, j, num));
            clauseNum += 36;
        }

    for (int i = 1;i<=7;i+=3)//(i,j)为每个盒子左上角坐标  九宫格约束
        for (int j = 1;j<=7;j+=3)
            for (char num = 1;num<=9;++num)
            {
                for (int x = 1; x <= 9; ++x) //盒子内有num
                    fprintf(fp, "%d ", pos_to_num(1, i + (x - 1) / 3, j + (x - 1) % 3, num));
                fprintf(fp, "0\n");
                clauseNum++;
                for (int m = 1;m<=8;++m)
                    for (int n = m + 1; n <= 9; ++n)
                        fprintf(fp, "-%d -%d 0\n", pos_to_num(1, i + (m - 1) / 3, j + (m - 1) % 3, num), pos_to_num(1, i + (n - 1) / 3, j + (n - 1) % 3, num));
                clauseNum += 36;

                for (int x = 1; x <= 9; ++x) //盒子内有num
                    fprintf(fp, "%d ", pos_to_num(2, i + (x - 1) / 3, j + (x - 1) % 3, num));
                fprintf(fp, "0\n");
                clauseNum++;
                for (int m = 1;m<=8;++m)
                    for (int n = m + 1; n <= 9; ++n)
                        fprintf(fp, "-%d -%d 0\n", pos_to_num(2, i + (m - 1) / 3, j + (m - 1) % 3, num), pos_to_num(2, i + (n - 1) / 3, j + (n - 1) % 3, num));
                clauseNum += 36;
            }

    fclose(fp);
    fp = fopen("temp", "r");
    FILE *out = fopen("sudoku.cnf", "w");
    fprintf(out, "p cnf %d %d\n", 1458, clauseNum);
    char str[1000];
    while(fgets(str,1000,fp)!=nullptr)
        fprintf(out, "%s", str);
    fclose(fp);
    fclose(out);
    remove("temp");
    return;
}

void sat_to_sudoku(char sudoku1[][10], char sudoku2[][10], int *result)
{
    for (int i = 1;i<=1458;++i)
    {
        if(result[i]>0)
        {
            if(i>729)
            {
                int pos = i - 729;
                sudoku2[(pos - 1) / 81 + 1][((pos - 1) % 81) / 9 + 1] = (pos - 1) % 9 + 1;
            }
            else
                sudoku1[(i - 1) / 81 + 1][((i - 1) % 81) / 9 + 1] = (i - 1) % 9 + 1;
        }
    }
    return;
}

int initial_legal_sukodu(char sudoku1[][10], char sudoku2[][10], int *result,int holes)
{
    srand((unsigned)time(nullptr));
    char num[10];
    for (int i = 0;i<10;++i)
        num[i] = i;
    for (int m = 1;m<=3;++m)//左上
    {
        for (int i = 9;i>=1;--i)//打乱
        {
            int t = rand() % i + 1;
            char temp = num[t];
            num[t] = num[i];
            num[i] = temp;
        }
        for (int i = 1;i<=9;++i)
            sudoku1[1 + (m - 1) * 3 + (i - 1) / 3][1 + (m - 1) * 3 + (i - 1) % 3] = num[i];
    }
    for (int i = 7;i<=9;++i)
        for (int j = 7;j<=9;++j)
            sudoku2[i - 6][j - 6] = sudoku1[i][j];
    for (int m = 2; m <= 3; ++m)
    {
        for (int i = 9;i>=1;--i)//打乱
        {
            int t = rand() % i + 1;
            char temp = num[t];
            num[t] = num[i];
            num[i] = temp;
        }
        for (int i = 1;i<=9;++i)
            sudoku2[1 + (m - 1) * 3 + (i - 1) / 3][1 + (m - 1) * 3 + (i - 1) % 3] = num[i];
    }
    sudoku_to_sat(sudoku1, sudoku2);

    char filename[20] = "sudoku.cnf";
    Solver *head=read_cnffile(filename);
    DPLL(head,vsids);
    for(int i=1;i<=head->var_num;++i)
        result[i]=head->trail[i].var;//保存解
    DestroySolver(head);
    head = nullptr;
    sat_to_sudoku(sudoku1, sudoku2, result); //获得初始布局

    int count = 0;
    int order[163];
    for (int i = 0;i<163;++i)
        order[i] = i;
    for (int i = 162;i>=1;--i)
    {
        int t = rand() % i + 1;
        int temp = order[t];
        order[t] = order[i];
        order[i] = temp;
    }
    int pos = 1;
    while(pos<=162&&count<holes)
    {
        int flag = order[pos] >= 82 ? 2 : 1;
        int target = flag == 2 ? (order[pos] - 81) : order[pos];
        pos++;
        int x = (target - 1) / 9 + 1, y = (target - 1) % 9 + 1;
        if((flag==1&&sudoku1[x][y]==0)||(flag==2&&sudoku2[x][y]==0))
            continue;
        char temp = flag == 2 ? sudoku2[x][y] : sudoku1[x][y];
        if(flag==1)
        {
            sudoku1[x][y] = 0;
            if(x>=7&&x<=9&&y>=7&&y<=9)
                sudoku2[x - 6][y - 6] = 0;
        }
        else
        {
            sudoku2[x][y] = 0;
            if(x>=1&&x<=3&&y>=1&&y<=3)
                sudoku1[x + 6][y + 6] = 0;
        }
        sudoku_to_sat(sudoku1, sudoku2);
        Solver *head=read_cnffile(filename);
        int is_single = 0;
        DPLL_sudoku(head, &is_single);
        if(is_single>1)
        {
            if(flag==1)
            {
                sudoku1[x][y] = temp;
                if(x>=7&&x<=9&&y>=7&&y<=9)
                    sudoku2[x - 6][y - 6] = temp;
            }
            else
            {
                sudoku2[x][y] = temp;
                if(x>=1&&x<=3&&y>=1&&y<=3)
                    sudoku1[x + 6][y + 6] = temp;
            }
        }
        else
            count++;
    }
    return count;
}

#endif
