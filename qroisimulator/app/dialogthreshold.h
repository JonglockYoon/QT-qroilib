#ifndef DIALOGTHRESHOLD_H
#define DIALOGTHRESHOLD_H

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
class DialogThreshold;
}

class DialogThreshold : public QDialog
{
    Q_OBJECT
public:
    DialogThreshold(QWidget *parent);
    ~DialogThreshold();

private slots:
    void setValuea1(int val);
    void setValuea2(int val);
    void setEditValuea1(const QString &);
    void setEditValuea2(const QString &);
    void changeRealtime(bool);
    void changeNot(bool);
    void activatedComboBoxSource(int act);
    void activatedComboBoxThresholdMethod(int);

    void setValueth1(int val);
    void setValueth2(int val);
    void setEditValueth1(const QString &val);
    void setEditValueth2(const QString &val);

    void setValueh1(int val);
    void setValueh2(int val);
    void setEditValueh1(const QString &val);
    void setEditValueh2(const QString &val);

    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

protected:
    void closeEvent(QCloseEvent *event);
public:
    void ExecThreshold(IplImage* iplImg);
public:
    int ThresholdRange(IplImage* grayImg, IplImage* outImg);
    double ThresholdOTSU(IplImage* grayImg, IplImage* outImg);
    int AdaptiveThreshold(IplImage* grayImg, IplImage* outImg);
    void AverageBrightInspection(IplImage* iplImg, IplImage* outImg);
    void ClaheTest(IplImage* iplImg, IplImage* outImg);

private:
    Ui::DialogThreshold *ui;

    bool bNot = false;
    bool bRealtime = false;
    int method;
    QString mName;
    int source = 0;

    IplImage* tmp = nullptr;
    int WinSize = 51;
    int AreaSize = 11;
    int low = 100;
    int high = 255;

    float clipLimit = 2.0;
    int tileGridSize = 8;
};

#endif // DIALOGTHRESHOLD_H
