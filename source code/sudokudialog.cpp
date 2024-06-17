#include "sudokudialog.h"
#include "ui_sudokudialog.h"
#include "dialog.h"
#include "basic.h"
#include <QString>
#include <QDialog>
#include <QIntValidator>
#include <QFile>
#include <QFileDialog>
#include <QTextEdit>
#include <QMessageBox>

SudokuDialog::SudokuDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SudokuDialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("双数独"));
    connect(ui->ReStart,SIGNAL(clicked()),this,SLOT(restart()));
    connect(ui->NewGame,SIGNAL(clicked()),this,SLOT(newgame()));
    connect(ui->OutPut,SIGNAL(clicked()),this,SLOT(output_cnf()));
    connect(ui->Answer,SIGNAL(clicked()),this,SLOT(show_answer()));
    int x=25,y=25;
    for(int i=0;i<15;++i)
        for(int j=0;j<15;++j)
        {
            box[i][j].setParent(this);
            box[i][j].setAlignment(Qt::AlignCenter);
            box[i][j].resize(50,50);
            box[i][j].setFont(QFont("Micrisoft YaHei",25,-1));
            box[i][j].move(x+j*50,y+i*50);
            if((i<=8&&j<=8)||(i<=14&&i>=6&&j<=14&&j>=6))
            {
                box[i][j].setValidator(new QIntValidator(0,9,this));
                box[i][j].setStyleSheet("QLineEdit:hover {background-color: lightblue}");
                connect(&box[i][j],SIGNAL(textChanged(QString)),this,SLOT(judge_win()));
                box[i][j].setDisabled(true);
            }
            else
            {
                box[i][j].setVisible(false);
                box[i][j].setDisabled(true);
            }
        }
    ui->HolesEdit->setPlaceholderText(tr("最大挖洞数(0-153)"));
    ui->HolesEdit->setValidator(new QIntValidator(0,110,this));
    memset(result,0,sizeof(int)*1459);
    //emit ui->NewGame->clicked();
}

SudokuDialog::~SudokuDialog()
{
    delete ui;
}

void SudokuDialog::restart()
{
    for(int i=0;i<15;++i)
        for(int j=0;j<15;++j)
        {
            if((i<=8&&j<=8)||(i<=14&&i>=6&&j<=14&&j>=6))
            {
                if(box[i][j].isEnabled()==true)
                    box[i][j].clear();
            }
        }
}

void SudokuDialog::newgame()
{
    //读取挖洞数HolesEdit
    bool ok1;
    int holes=ui->HolesEdit->text().toInt(&ok1);
    if(ok1==false||holes<=0||holes>153)
    {
        QMessageBox::warning(this,tr("警告"),tr("请按正确格式输入数字！"),QMessageBox::Ok);
        return;
    }
    memset(result,0,sizeof(int)*1459);
    result[0]=1458;
    for(int i=0;i<15;++i)
        for(int j=0;j<15;++j)
        {
            if((i<=8&&j<=8)||(i<=14&&i>=6&&j<=14&&j>=6))
            {
                box[i][j].setEnabled(true);
                box[i][j].clear();
            }
        }
    char s1[10][10],s2[10][10];
    memset(s1,0,sizeof(char)*100);
    memset(s2,0,sizeof(char)*100);


    clock_t begin,end;
    begin=clock();
    int final_holes=initial_legal_sukodu(s1,s2,result,holes);

    end=clock();
    double dtime=(double)(end-begin)/CLOCKS_PER_SEC;
    QString out;
    out+=tr("挖洞数:");
    out+=QString::number(final_holes);
    out+=tr("用时:");
    out+=QString::number(dtime,'f',3);
    out+=tr("s");
    QMessageBox::information(this,tr("提示"),out,QMessageBox::Ok);


    for(int i=0;i<15;++i)
        for(int j=0;j<15;++j)
        {
            if(i<=8&&j<=8)//i+1 j+1
            {
                if(s1[i+1][j+1]!=0)
                {
                    box[i][j].setText(QString::number(s1[i+1][j+1]));
                    box[i][j].setDisabled(true);
                }
                else box[i][j].setEnabled(true);

            }
            if(((i>=9&&i<=14)&&(j>=6&&j<=14))||((i>=5&&i<=8)&&(j>=9&&j<=14)))//i-5 j-5
            {
                if(s2[i-5][j-5]!=0)
                {
                    box[i][j].clear();
                    box[i][j].setText(QString::number(s2[i-5][j-5]));
                    box[i][j].setDisabled(true);
                }
                else box[i][j].setEnabled(true);
            }
        }
}

void SudokuDialog::output_cnf()
{
    char s1[10][10],s2[10][10];
    memset(s1,0,sizeof(char)*100);
    memset(s2,0,sizeof(char)*100);
    for(int i=0;i<15;++i)
        for(int j=0;j<15;++j)
        {
            if(i<=8&&j<=8)
            {
                bool ok1;
                s1[i+1][j+1]=box[i][j].text().toInt(&ok1);
                //if(ok1==false) QMessageBox::warning(this,tr("警告"),tr("输出时出错！"),QMessageBox::Ok);
                if(ok1==false) s1[i+1][j+1]=0;
            }
            if(i<=14&&i>=6&&j<=14&&j>=6)
            {
                bool ok1;
                s2[i-5][j-5]=box[i][j].text().toInt(&ok1);
                //if(ok1==false) QMessageBox::warning(this,tr("警告"),tr("输出时出错！"),QMessageBox::Ok);
                if(ok1==false) s2[i-5][j-5]=0;
            }
        }
    sudoku_to_sat(s1,s2);
    QMessageBox::information(this,tr("提示"),tr("输出成功！"),QMessageBox::Ok);
}


void SudokuDialog::show_answer()
{
    char s1[10][10],s2[10][10];
    memset(s1,0,sizeof(char)*100);
    memset(s2,0,sizeof(char)*100);
    sat_to_sudoku(s1,s2,result);
    for(int i=0;i<15;++i)
        for(int j=0;j<15;++j)
        {
            if(i<=8&&j<=8)
            {
                if(box[i][j].isEnabled()==true)
                {
                    QString str=QString::number(s1[i+1][j+1]);
                    box[i][j].setText(str);
                    box[i][j].setDisabled(true);
                }
            }
            if(i<=14&&i>=6&&j<=14&&j>=6)
            {
                if(box[i][j].isEnabled()==true)
                {
                    QString str=QString::number(s2[i-5][j-5]);
                    box[i][j].setText(str);
                    box[i][j].setDisabled(true);
                }
            }
        }
}

void SudokuDialog::judge_win()
{
    char s1[10][10],s2[10][10];
    int flag=0;
    memset(s1,0,sizeof(char)*100);
    memset(s2,0,sizeof(char)*100);
    sat_to_sudoku(s1,s2,result);
    for(int i=0;i<15;++i)
        for(int j=0;j<15;++j)
        {
            if(i<=8&&j<=8)
            {
                if(box[i][j].isEnabled()==true)
                {
                    QString str=QString::number(s1[i+1][j+1]);
                    QString sstr=box[i][j].text();
                    if(str!=sstr)
                    {
                        box[i][j].setStyleSheet("QLineEdit {background-color: red}");
                        flag=1;
                    }
                    else box[i][j].setStyleSheet("QLineEdit {background-color: white}");
                }
            }
            if(i<=14&&i>=6&&j<=14&&j>=6)
            {
                if(box[i][j].isEnabled()==true)
                {
                    QString str=QString::number(s2[i-5][j-5]);
                    QString sstr=box[i][j].text();
                    if(str!=sstr)
                    {
                        box[i][j].setStyleSheet("QLineEdit {background-color: red}");
                        flag=1;
                    }
                    else box[i][j].setStyleSheet("QLineEdit {background-color: white}");
                }
            }
        }
    if(!flag) QMessageBox::information(this,tr("提示"),tr("数独已完成!"),QMessageBox::Ok);
}

