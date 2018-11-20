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
#include <opencv2/xfeatures2d/nonfree.hpp>
#include "voronoithinner.h"

namespace Ui {
class DialogApplication;
}

struct SURFDetector
{
    cv::Ptr<cv::Feature2D> surf;
    SURFDetector(double hessian = 800.0)
    {
        surf = cv::xfeatures2d::SURF::create(hessian);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        surf->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};
struct SIFTDetector
{
    cv::Ptr<cv::Feature2D> sift;
    SIFTDetector(int nFeatures = 0)
    {
        sift = cv::xfeatures2d::SIFT::create(nFeatures);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        sift->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};
struct ORBDetector
{
    cv::Ptr<cv::Feature2D> orb;
    ORBDetector(int nFeatures = 500)
    {
        orb = cv::ORB::create(nFeatures);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        orb->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};

class DialogApplication : public QDialog
{
    Q_OBJECT
public:
    DialogApplication(QWidget *parent);
    ~DialogApplication();

private slots:
    void changeRealtime(bool);
    void activatedComboBoxSource0(int act);
    void activatedComboBoxSource1(int act);
    void activatedComboBoxDetector(int act);
    void on_pushButtonClose_clicked();
    void on_pushButtonExec_clicked();
    void updatePlayerUI(const QImage* img);

    void on_radioButtonCenterOfPlusMark_clicked();

    void on_radioButtonCodeScanner_clicked();

    void on_radioButtonGeoMatch_clicked();

    void on_radioButtonXFeatures2D_clicked();

    void on_radioButtonColorDetect_clicked();

    void on_radioButtonFFT_clicked();

    void on_radioButtonImgSeg_clicked();

    void on_radioButtonStitching_clicked();

protected:
    void closeEvent(QCloseEvent *event);
public:
    void DrawMark(IplImage* iplImg, int x, int y);
    void ExecApplication(IplImage* iplImg, IplImage* iplImg2);
    void ExecRansacLinefit(IplImage* iplImg, int offset);

    IplImage* backImg = nullptr;
    void restoreLoadedImage();
    cv::Mat convertFFTMag();
    cv::Mat fImage;
    cv::Mat ftI;
    cv::Mat displayImage;
    void FFTTest();
    void StitchingTest(IplImage* iplImg, IplImage* iplImg2);
    void FilterHueBoundary(cv::Mat& input);
    void ImageSegmentationCard(IplImage* iplImg);
    void ImageSegmentationCoin(IplImage* iplImg);
    void ImageSegmentationLineWidth(IplImage* iplImg);

public:
    void centerOfPlusmaek(IplImage* iplImg);
    void codeScanner(IplImage* iplImg);
    void ExecGeoMatch(IplImage* iplImg, IplImage* iplImg2);

    int xfeatureTest(IplImage* iplImg, IplImage* iplImg2);
    cv::Mat drawGoodMatches(
        const cv::Mat& img1,
        const cv::Mat& img2,
        const std::vector<cv::KeyPoint>& keypoints1,
        const std::vector<cv::KeyPoint>& keypoints2,
        std::vector<cv::DMatch>& matches,
        std::vector<cv::Point2f>& scene_corners_
    );

    void ColorDetect(IplImage* iplImg, IplImage* iplImg2);

private:
    VoronoiThinner thinner;
    Ui::DialogApplication *ui;
    QString mName;
    int method;
    int source0 = 0;
    int source1 = 0;
    bool bRealtime = false;

    IplImage* outImg = nullptr;

    CvPoint plusmarkpt[4];

};

#endif // DIALOGapp_H
