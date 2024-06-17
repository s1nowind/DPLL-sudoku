#ifndef SATSOLVERDIALOG_H
#define SATSOLVERDIALOG_H

#include <QDialog>

namespace Ui {
class SATSolverDialog;
}

class SATSolverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SATSolverDialog(QWidget *parent = nullptr);
    ~SATSolverDialog();

private slots:
    void analyse_cnf(void);
    void DPLL_solver1(void);
    void DPLL_solver2(void);

private:
    Ui::SATSolverDialog *ui;
};

#endif // SATSOLVERDIALOG_H
