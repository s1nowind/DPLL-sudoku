#ifndef BASIC_H
#define BASIC_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#define TRUE 1
#define FALSE -1
#define ERROR -2
#define UNKNOWN 0
#define UP_TIME_MS 100000.0
#define MAX_LEN 2000
#define MAX_VAR_NUM 2000
#define CLAUSE 0//标记回溯栈
#define VARIABLE 1
#define DEC 1//标记迹
#define NOTDEC 0
#define MDEC 0
#define VSIDSCOUNT 80//DPLL
#define LEARN_MAXLENGTH 20

#define ABS(x) ((x)>0?(x):(0-(x)))
#define ASSIGN(x) ((x)>0?1:-1)

typedef int status;
typedef char boolean;

typedef struct variable{
    int var;
    struct variable *next;
} Variable;


typedef struct clause{
    status is_delete;//是否被删除
    int length;//子句长度
    struct variable *first;//第一个未删除的文字
    struct variable *rmfirst;//第一个删除的文字
    struct clause *next, *pre;//前后子句
} Clause;


typedef struct variableIndex{
    struct variableIndex *next;//改文字文字的索引
    Variable *var;//指向该文字
    Clause *varClause;//文字所在子句
} VarIndex;

typedef struct tr{
    int tag;
    int level;
    int var;
}Trail;

typedef struct Solver {
    int var_num;  //文字个数
    int clause_num;  //子句个数
    int choose_count;

    int *count;  //VSIDS方法决策数组

    int *trailOrder;//按顺序储存迹中变量(从1开始)，方便trail数组直接随机读取迹的内容
    struct tr *trail;//迹的内容

    struct variableIndex *var_index;    //指向文字索引

    struct clause *root;  //指向第一个子句
} Solver;

enum strategy{//策略选择
    mom,
    vsids,
    first,
};
typedef struct change{//回溯栈
    int tag;
    Clause *cla;
    Variable *vari;
    int level;
    struct change *next;
} ChangeStack;

//文字处理
status AddVar(Clause *p, int target);//增加
Variable *RemoveVar(Clause *p, int target);//移除
status RecoverVar(Clause *p, Variable *target);//恢复


//文字索引
status AddClauseIndex(Solver *head, Clause *p);
status CreateVarIndex(Solver *head);
status DestroyClauseIndex(Solver *head, Clause *p);//删除子句的文字索引
status DestroyAllIndex(Solver *head);

//子句处理
status AddClause(Solver *head, int *clause, int length);
status InsertClause(Solver *head, Clause *target);

Clause *RemoveClause(Solver *head, Clause *p);//移除
status DestroyClause(Solver *head, Clause *p); //销毁
status RecoverClause(Solver *head, Clause *p);
status DestroyAllClause(Solver *head);

//boolean IsEmptyClause(Solver *head);//判断子句集是否为空
//boolean IsUnitClause(Clause *p);//是否为单子句
//boolean HaveEmptyclause(Solver *head);//是否有空子句
Clause *GetUnitClause(Solver *head);

//求解
int CountPos(Solver *head, int var); // count数组中变量计数器位置(正负)
status CreateSolver(Solver *head, int var_num, int clause_num);
status DestroySolver(Solver *head);

//show
//status print_trail(Solver *head);
//status check_clause(Solver *head);
//cnfparser

Solver *read_cnffile(char filepath[]);
status save_resfile(char filepath[], status state, Solver *head, double time);
status save_cnffile(char filepath[], Solver *head);


//DPLL_solver
//策略
int MOM_strategy(Solver *head);
int VSIDS_strategy(Solver *head);
int First_strategy(Solver *head);
int Choose_strategy(Solver *head, enum strategy s);

//BCP
status BCP(Solver *head, int level, int var, int tag, ChangeStack *sk);
//辅助
status SaveChange(Clause *cla, Variable *vari, int tag, ChangeStack *sk, int level);
//CDCL相关函数，参数未定
status BackJump(Solver *head, int back, ChangeStack *sk);
status SaveTrail(Solver *head, int tag, int var, int level); //同时保存层级
int BackTrail(Solver *head, int level);
//DPLL
boolean DPLL(Solver *head,enum strategy s);
boolean DPLL_sudoku(Solver *head,int *num);//判断解的个数


//sudoku
//使用字符数组仅为节省空间，不直接存字符，当作数字来看
int pos_to_num(int x, int a, int b, char num);
//x指哪个区域的数独 num指这个空的数字，a和b指横纵位置

void sudoku_to_sat(char sudoku1[][10], char sudoku2[][10]);
void sat_to_sudoku(char sudoku1[][10], char sudoku2[][10], int *result);


int initial_legal_sukodu(char sudoku1[][10], char sudoku2[][10], int *result,int holes);//创建挖洞后的数独，并通过result传回完整答案

#endif // BASIC_H
