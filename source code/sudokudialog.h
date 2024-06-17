#ifndef SUDOKUDIALOG_H
#define SUDOKUDIALOG_H

#include <QDialog>
#include <QLineEdit>

namespace Ui {
class SudokuDialog;
}

class SudokuDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SudokuDialog(QWidget *parent = nullptr);
    ~SudokuDialog();

private slots:
    void restart(void);
    void newgame(void);
    void show_answer(void);
    void output_cnf(void);
    void judge_win(void);

private:
    Ui::SudokuDialog *ui;
    int result[1459];
    QLineEdit box[15][15];
};

#endif // SUDOKUDIALOG_H
