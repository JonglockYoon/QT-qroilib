#include "dialogapplication.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogapplication.h"
#include "imgprocbase.h"
#include "common.h"


DialogApplication::DialogApplication(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogApplication)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Application%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    method = 0;
    ui->setupUi(this);

    ui->radioButtonStereoBM->setChecked(true);

    //bm = StereoBM::create(16,9);
    //sgbm = StereoSGBM::create(0,16,3);

    ndisparities = 16*5;   /**< Range of disparity */
    int SADWindowSize = 21; /**< Size of the block window. Must be odd */
    bm = cv::StereoBM::create( ndisparities, SADWindowSize );
    sgbm = cv::StereoSGBM::create(0,    //int minDisparity
            96,     //int numDisparities
            5,      //int SADWindowSize
            600,    //int P1 = 0
            2400,   //int P2 = 0
            10,     //int disp12MaxDiff = 0
            16,     //int preFilterCap = 0
            2,      //int uniquenessRatio = 0
            20,    //int speckleWindowSize = 0
            30,     //int speckleRange = 0
            true);  //bool fullDP = false

}

DialogApplication::~DialogApplication()
{
}

void DialogApplication::closeEvent(QCloseEvent *event)
{
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        if (pView) {
            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }


    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
}


void DialogApplication::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}

//
// bRealtime 처리..
//
void DialogApplication::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogApplication::on_pushButtonExec_clicked()
{
    IplImage* iplImg1 = nullptr;
    IplImage* iplImg2 = nullptr;
    ViewMainPage *viewMain = (ViewMainPage *)theMainWindow->viewMainPage();
    if (viewMain) {
        iplImg1 = viewMain->getIplgray(0);
        iplImg2 = viewMain->getIplgray(1);
        if (!iplImg1 || !iplImg2)
            return;

        ExecApplication(iplImg1, iplImg2);
    }
}

void DialogApplication::on_radioButtonStereoBM_clicked()
{
    method = 0;
}

void DialogApplication::on_radioButtonStereoSGBM_clicked()
{
    method = 1;
}

void DialogApplication::ExecApplication(IplImage* iplImg1, IplImage* iplImg2)
{
    if (tmp) {
        if (tmp->width != iplImg1->width || tmp->height != iplImg1->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }

    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg1->width, iplImg1->height), iplImg1->depth, iplImg1->nChannels);
    //cvCopy(iplImg, tmp);
    cvZero(tmp);

    switch(method)
    {
    case 0: // BM
        StereoBM(iplImg1, iplImg2);
        break;
    case 1: // SGBM
        StereoSGBM(iplImg1, iplImg2);
        break;
    }
    theMainWindow->outWidget(mName, tmp);
}

static void saveXYZ(const cv::Mat& mat)
{
    const double max_z = 1.0e4;
    //FILE* fp = fopen(filename, "wt");
    for(int y = 0; y < mat.rows; y++)
    {
        for(int x = 0; x < mat.cols; x++)
        {
            cv::Vec3f point = mat.at<cv::Vec3f>(y, x);
            if(fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
            qDebug() << point[0] << point[1] << point[2];
        }
    }
    //fclose(fp);
}


void DialogApplication::StereoBM(IplImage* iplImg1, IplImage* iplImg2)
{

//    int SADWindowSize = 9, ndisparities = 112;
//    bm->setPreFilterCap(31);
//    bm->setBlockSize(SADWindowSize > 0 ? SADWindowSize : 9);
//    bm->setMinDisparity(0);
//    bm->setNumDisparities(ndisparities);
//    bm->setTextureThreshold(10);
//    bm->setUniquenessRatio(15);
//    bm->setSpeckleWindowSize(100);
//    bm->setSpeckleRange(32);
//    bm->setDisp12MaxDiff(1);

    cv::Mat img1 = cv::cvarrToMat(iplImg1);
    cv::Mat img2 = cv::cvarrToMat(iplImg2);
    cv::Mat disp;
    cv::Mat disp8;

    //imshow("left", img1);
    //imshow("right", img2);

    bm->compute(img1, img2, disp);
    double minVal; double maxVal;
    minMaxLoc( disp, &minVal, &maxVal );

    //disp.convertTo(disp8, CV_8U, 255/(ndisparities*16.));
    disp.convertTo( disp8, CV_8UC1, 255/(maxVal - minVal));

    //imshow("disp", disp8);

    IplImage ipl = disp8;
    cvCopy(&ipl, tmp);

//    Mat Q;
//    cv::Mat xyz(ndisparities,CV_32FC3);
//    reprojectImageTo3D(disp, xyz, Q, true);
//    saveXYZ(xyz);
}
void DialogApplication::StereoSGBM(IplImage* iplImg1, IplImage* iplImg2)
{

//    int cn = iplImg1->nChannels;
//    int SADWindowSize = 9;
//    ndisparities = 112;
//    sgbm->setPreFilterCap(63);
//    int sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
//    sgbm->setBlockSize(sgbmWinSize);

//    enum { STEREO_BM=0, STEREO_SGBM=1, STEREO_HH=2, STEREO_VAR=3, STEREO_3WAY=4 };
//    int alg = STEREO_SGBM;

//    sgbm->setP1(8*cn*sgbmWinSize*sgbmWinSize);
//    sgbm->setP2(32*cn*sgbmWinSize*sgbmWinSize);
//    sgbm->setMinDisparity(0);
//    sgbm->setNumDisparities(ndisparities);
//    sgbm->setUniquenessRatio(10);
//    sgbm->setSpeckleWindowSize(100);
//    sgbm->setSpeckleRange(32);
//    sgbm->setDisp12MaxDiff(1);
//    if(alg==STEREO_HH)
//        sgbm->setMode(StereoSGBM::MODE_HH);
//    else if(alg==STEREO_SGBM)
//        sgbm->setMode(StereoSGBM::MODE_SGBM);
//    else if(alg==STEREO_3WAY)
//        sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);

    cv::Mat img1 = cv::cvarrToMat(iplImg1);
    cv::Mat img2 = cv::cvarrToMat(iplImg2);
    cv::Mat disp;
    cv::Mat disp8;

    sgbm->compute(img1, img2, disp);

    //if( alg != STEREO_VAR )
        disp.convertTo(disp8, CV_8U, 255/(ndisparities*16.));
    //else
    //    disp.convertTo(disp8, CV_8U);

    IplImage ipl = disp8;
    cvCopy(&ipl, tmp);
}
