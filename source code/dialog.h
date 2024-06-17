#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void CreateSudokuDialog(void);//创建数独界面
    void SolveSATDialog(void);//创建SAT求解器界面



private:
    Ui::Dialog *ui;
};
#endif // DIALOG_H
