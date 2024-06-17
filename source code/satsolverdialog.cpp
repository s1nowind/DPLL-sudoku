#include "satsolverdialog.h"
#include "ui_satsolverdialog.h"
#include "dialog.h"
#include "basic.h"
#include <QString>
#include <QDialog>
#include <QFileDialog>
#include <QTextEdit>
#include <QMessageBox>

SATSolverDialog::SATSolverDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SATSolverDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("SAT求解"));
    connect(ui->AnalyseCnf,SIGNAL(clicked()),this,SLOT(analyse_cnf()));
    connect(ui->After,SIGNAL(clicked()),this,SLOT(DPLL_solver1()));
    connect(ui->Before,SIGNAL(clicked()),this,SLOT(DPLL_solver2()));
}

SATSolverDialog::~SATSolverDialog()
{
    delete ui;
}

void SATSolverDialog::analyse_cnf()//只有cnf文件解析
{
    QDialog *dialog=new Dialog(this);
    QString filename=QFileDialog::getOpenFileName(this,tr("Open File"),".",tr(".cnf文件(*.cnf)"));
    if(!filename.isEmpty())//是否选择了文件
    {
        char name[1000];
        strcpy(name,filename.toStdString().c_str());
        Solver *head=read_cnffile(name);
        if(head==nullptr)
        {
            QMessageBox::warning(this,tr("错误"),tr("读取文件时发生错误!"),QMessageBox::Ok);
            return;
        }
        dialog->resize(800,600);
        QTextEdit *editText=new QTextEdit(dialog);
        editText->resize(700,550);
        editText->move(50,50);
        dialog->show();
        QString str;//cstr与Qstr转换
        //解析
        Clause *p=head->root;
        while(p->next!=nullptr)
        {
            Variable *vp=p->first;
            str+="(";
            str+=QString::number(vp->var);
            vp=vp->next;
            while(vp!=nullptr)
            {
                str+="∪";
                str+=QString::number(vp->var);
                vp=vp->next;
            }
            str+=")∩\n";
            p=p->next;
        }
        str+="(";
        Variable *vp=p->first;
        str+=QString::number(vp->var);
        vp=vp->next;
        while(vp!=nullptr)
        {
            str+="∪";
            str+=QString::number(vp->var);
            vp=vp->next;
        }
        str+=")\n";
        DestroySolver(head);
        editText->setText(str);
    }
    return;
}

void SATSolverDialog::DPLL_solver1()
{
    QString filename=QFileDialog::getOpenFileName(this,tr("Open File"),".",tr(".cnf文件(*.cnf)"));
    if(!filename.isEmpty())
    {
        char name[1000];
        strcpy(name,filename.toStdString().c_str());
        Solver *head=read_cnffile(name);
        if(head==nullptr)
        {
            QMessageBox::warning(this,tr("错误"),tr("读取文件时发生错误!"),QMessageBox::Ok);
            return;
        }
        clock_t begin,end;
        begin=clock();
        int res=DPLL(head,mom);
        end=clock();
        double dtime=(double)(end-begin)/CLOCKS_PER_SEC*1000.0;
        QString out;

        if(res==FALSE)
        {
            out+=tr("不可满足！运算耗时: ");
            out+=QString::number(dtime,'f',3);
            out+=tr("ms\n结果将输出至同目录下");
            QMessageBox::information(this,tr("计算结果"),out,QMessageBox::Ok);
        }
        else if(res==TRUE)
        {
            out+=tr("可以满足！运算耗时: ");
            out+=QString::number(dtime,'f',3);
            out+=tr("ms\n结果将输出至同目录下");
            QMessageBox::information(this,tr("计算结果"),out,QMessageBox::Ok);
        }
        save_resfile(name,res,head,dtime);
        DestroySolver(head);
    }
    return;
}

void SATSolverDialog::DPLL_solver2()
{
    QMessageBox::information(this,tr("提示"),tr("该方法比较适合处理数独类问题"),QMessageBox::Ok);
    QString filename=QFileDialog::getOpenFileName(this,tr("Open File"),".",tr(".cnf文件(*.cnf)"));
    if(!filename.isEmpty())
    {
        char name[1000];
        strcpy(name,filename.toStdString().c_str());
        Solver *head=read_cnffile(name);
        if(head==nullptr)
        {
            QMessageBox::warning(this,tr("错误"),tr("读取文件时发生错误!"),QMessageBox::Ok);
            return;
        }
        clock_t begin,end;
        begin=clock();
        int res=DPLL(head,vsids);
        end=clock();
        double dtime=(double)(end-begin)/CLOCKS_PER_SEC*1000.0;
        QString out;

        if(res==FALSE)
        {
            out+=tr("不可满足！运算耗时: ");
            out+=QString::number(dtime,'f',3);
            out+=tr("ms\n结果将输出至同目录下");
            QMessageBox::information(this,tr("计算结果"),out,QMessageBox::Ok);
        }
        else if(res==TRUE)
        {
            out+=tr("可以满足！运算耗时: ");
            out+=QString::number(dtime,'f',3);
            out+=tr("ms\n结果将输出至同目录下");
            QMessageBox::information(this,tr("计算结果"),out,QMessageBox::Ok);
        }
        save_resfile(name,res,head,dtime);
        DestroySolver(head);
    }
    return;
}
