#include "dialog.h"
#include "satsolverdialog.h"
#include "sudokudialog.h"
#include "qobjectdefs.h"
#include "ui_dialog.h"
#include <QDialog>
#include <QString>


Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("菜单界面"));

   connect(ui->Sudoku,SIGNAL(clicked()),this,SLOT(CreateSudokuDialog()));
   connect(ui->SatSolver,SIGNAL(clicked()),this,SLOT(SolveSATDialog()));
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::CreateSudokuDialog()
{
    SudokuDialog *sudoku_dialog=new SudokuDialog(this);
    sudoku_dialog->show();
}

void Dialog::SolveSATDialog()
{
    SATSolverDialog *sat_dialog=new SATSolverDialog(this);
    sat_dialog->show();
}
