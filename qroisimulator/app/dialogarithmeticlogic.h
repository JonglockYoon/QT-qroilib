#ifndef DIALOGarithmeticlogic_H
#define DIALOGarithmeticlogic_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"

namespace Ui {
class DialogArithmeticlogic;
}

class DialogArithmeticlogic : public QDialog
{
    Q_OBJECT
public:
    DialogArithmeticlogic(QWidget *parent);
    ~DialogArithmeticlogic();

private slots:
    void changeRealtime(bool);
    void activatedComboBoxSource0(int act);
    void activatedComboBoxSource1(int act);
    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

    void on_radioButtonCopy_clicked();

    void on_radioButtonInvert_clicked();

    void on_radioButtonAdd_clicked();

    void on_radioButtonSubtract_clicked();

    void on_radioButtonAnd_clicked();

    void on_radioButtonOr_clicked();

    void on_radioButtonXor_clicked();

protected:
    void closeEvent(QCloseEvent *event);
public:
    void ExecArithmeticlogic(IplImage* iplImg);
public:
private:
    Ui::DialogArithmeticlogic *ui;
    QString mName;
    int source0 = 0;
    int source1 = 0;
    bool bRealtime = false;
    int method = 0;

    IplImage* tmp = nullptr;
};

#endif // DIALOGarithmeticlogic_H
