#ifndef DIALOGlinefit_H
#define DIALOGlinefit_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"

namespace Ui {
class DialogLinefit;
}

class DialogLinefit : public QDialog
{
    Q_OBJECT
public:
    DialogLinefit(QWidget *parent);
    ~DialogLinefit();

private slots:
    void changeRealtime(bool);
    void activatedComboBoxSource(int act);
    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

protected:
    void closeEvent(QCloseEvent *event);
public:
    void ExecRansacLinefit(IplImage* iplImg);
public:
private:
    Ui::DialogLinefit *ui;
    QString mName;
    int source = 0;
    bool bRealtime = false;

    IplImage* tmp = nullptr;
};

#endif // DIALOGlinefit_H
