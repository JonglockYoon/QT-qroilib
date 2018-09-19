#ifndef DIALOGCONTOUR_H
#define DIALOGCONTOUR_H

#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QStringListModel>
#include "imgprocbase.h"

namespace Ui {
class DialogContour;
}

class DialogContour : public QDialog
{
    Q_OBJECT
public:
    DialogContour(QWidget *parent);
    ~DialogContour();

private slots:
    void changeRealtime(bool);
    void activatedComboBoxSource0(int act);
    void activatedComboBoxSource1(int act);
    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);
    void changeApproxPoly(bool);
    void setEditEPS(const QString &);
    void setEditMinArea(const QString &);

    void on_radioButtonDrawContour_clicked();

    void on_radioButtonMatchShapes_clicked();

    void on_radioButtonShapeContextDistanceExtractor_clicked();

    void on_radioButtonDrawContourRect_clicked();

protected:
    void closeEvent(QCloseEvent *event);
public:
    void DrawResultCrossMark(IplImage *iplImage, cv::Point2f center);
    void ExecContour(IplImage* iplImg, IplImage* iplImg2);
private:
    int method = 0;
    int bContourRect = 0;
    Ui::DialogContour *ui;
    QString mName;
    int source0 = 0;
    int source1 = 0;
    bool bRealtime = false;
    bool bApproxPoly = false;
    double dEPS = 0.01;
    double dMinArea = 0;
    IplImage* outImg = nullptr;

    void DrawContour(IplImage* iplImg);
    void TestMatchShapes(IplImage* iplImg, IplImage* iplImg2);
    void TestShapeContextDistanceExtractor(IplImage* iplImg, IplImage* iplImg2);

    vector<cv::Point> sampleContour(const cv::Mat& image, int n = 300) ;
    int main1() ;
};

#endif // DIALOGCONTOUR_H
