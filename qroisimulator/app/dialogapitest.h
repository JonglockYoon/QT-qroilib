#ifndef DIALOGapiTest_H
#define DIALOGapiTest_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"
#include <opencv2/xfeatures2d/nonfree.hpp>

namespace Ui {
class DialogApiTest;
}

class DialogApiTest : public QDialog
{
    Q_OBJECT
public:
    DialogApiTest(QWidget *parent);
    ~DialogApiTest();

private slots:
    void changeRealtime(bool);
    void activatedComboBoxSource0(int act);
    void activatedComboBoxSource1(int act);
    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

    void on_radioButtoncopyMakeBorder_clicked();

protected:
    void closeEvent(QCloseEvent *event);

public:
    void ExecApplication(IplImage* iplImg, IplImage* iplImg2);
    void copyMakeBorderTest(IplImage* iplImg);

private:
    Ui::DialogApiTest *ui;
    QString mName;
    int method;
    int source0 = 0;
    int source1 = 0;
    bool bRealtime = false;

    IplImage* outImg = nullptr;

};

#endif // DIALOGapiTest_H
