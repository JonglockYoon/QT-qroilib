#ifndef DIALOGapp_H
#define DIALOGapp_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"

namespace Ui {
class DialogApplication;
}

class DialogApplication : public QDialog
{
    Q_OBJECT
public:
    DialogApplication(QWidget *parent);
    ~DialogApplication();

private slots:

    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

    void on_radioButtonStereoBM_clicked();
    void on_radioButtonStereoSGBM_clicked();

protected:
    void closeEvent(QCloseEvent *event);
public:
    void ExecApplication(IplImage* iplImg1, IplImage* iplImg2);
    void StereoBM(IplImage* iplImg1, IplImage* iplImg2);
    void StereoSGBM(IplImage* iplImg1, IplImage* iplImg2);

private:
    Ui::DialogApplication *ui;
    QString mName;
    int method;


    IplImage* tmp = nullptr;

    int ndisparities;

    cv::Ptr<cv::StereoBM> bm;
    cv::Ptr<cv::StereoSGBM> sgbm;

};

#endif // DIALOGapp_H
