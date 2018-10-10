#include "dialogapplication.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogapplication.h"
#include "imgprocbase.h"
#include "common.h"
#include "QZXing.h"
#include <zxing/NotFoundException.h>
#include <zxing/ReaderException.h>
#include "geomatch.h"

using namespace zxing;
using namespace cv;
using namespace cv::xfeatures2d;

DialogApplication::DialogApplication(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogApplication)
{
    ui->setupUi(this);
    ui->editGeoLow->setText("10");
    ui->editGeoHigh->setText("100");
    ui->editGeoScore->setText("0.9");
    ui->editGeoGreediness->setText("0.5");

    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Application%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    outImg = nullptr;
    method = 0;

    ui->comboBoxSource0->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource0->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource0->setCurrentIndex(0);
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource1->insertItem(i, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource1->setCurrentIndex(0);


    connect(ui->comboBoxSource0, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource0(int)));
    connect(ui->comboBoxSource1, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource1(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));

    ui->radioButtonCenterOfPlusMark->setChecked(true);

    ui->comboBoxDetector->insertItem(0, QIcon(), QString::fromLocal8Bit("SURF"));
    ui->comboBoxDetector->insertItem(1, QIcon(), QString::fromLocal8Bit("SIFT"));
    ui->comboBoxDetector->insertItem(2, QIcon(), QString::fromLocal8Bit("ORB"));
    ui->comboBoxDetector->setCurrentIndex(0);
    connect(ui->comboBoxDetector, SIGNAL(activated(int)), this, SLOT(activatedComboBoxDetector(int)));

    ui->editParam1->setText("800");
    ui->editDistanceCoef->setText("20.0");
    ui->editMaxMatchingSize->setText("100");


    ui->editHue1->setText("160"); // red
    ui->editHue2->setText("30");
    ui->editSaturation1->setText("50");
    ui->editSaturation2->setText("255");

}

DialogApplication::~DialogApplication()
{
    if (backImg)
        cvReleaseImage(&backImg);
}

void DialogApplication::closeEvent(QCloseEvent *event)
{
//    if (source == 0)
//    {
//        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
//        if (pView) {
//            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
//        }
//    } else {
//        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
//        ViewOutPage* pView = pWidget->mViewOutPage;
//        if (pView) {
//            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
//        }
//    }

    if (outImg)
        cvReleaseImage(&outImg);
    outImg = nullptr;
}

void DialogApplication::changeRealtime(bool checked)
{
    bRealtime = checked;

    if (source0 == 0)
    {
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    }
}

void DialogApplication::activatedComboBoxSource0(int act)
{
    source0 = act;

    ui->comboBoxSource0->clear();
    ui->comboBoxSource0->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource0->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource0->setCurrentIndex(act);
}
void DialogApplication::activatedComboBoxSource1(int act)
{
    source1 = act;

    ui->comboBoxSource1->clear();
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource1->insertItem(i, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource1->setCurrentIndex(act);
}
void DialogApplication::activatedComboBoxDetector(int act)
{
    if (act == 0) // SURF
    {
        ui->editParam1->setText("800");
        ui->editDistanceCoef->setText("5.0");
        ui->editMaxMatchingSize->setText("200");
    }
    else if (act == 1) // SIFT
    {
        ui->editParam1->setText("0");
        ui->editDistanceCoef->setText("10.0");
        ui->editMaxMatchingSize->setText("200");
    }
    else if (act == 2) // ORB
    {
        ui->editParam1->setText("2000");
        ui->editDistanceCoef->setText("5.0");
        ui->editMaxMatchingSize->setText("200");
    }
}
void DialogApplication::on_pushButtonClose_clicked()
{
    if (outImg)
        cvReleaseImage(&outImg);
    outImg = nullptr;
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
    IplImage* iplImg2 = nullptr;
    if (theMainWindow->vecOutWidget.size() > source1)
    {
        OutWidget* pWidget1 = theMainWindow->vecOutWidget[source1];
        ViewOutPage* pView1 = pWidget1->mViewOutPage;
        if (pView1) {
            iplImg2 = pView1->getIplgray();
        }
    }

    IplImage* iplImg = nullptr;
    if (source0 == 0)
    {
        ViewMainPage *viewMain = (ViewMainPage *)theMainWindow->viewMainPage();
        if (viewMain) {
            if (method == 4)
                iplImg = viewMain->getIplcolor();
            else
                iplImg = viewMain->getIplgray();

            if (!iplImg)
                return;


            Qroilib::DocumentView* v = viewMain->currentView();
            if (v != nullptr) {
                RoiObject *pData = nullptr;
                for (const Layer *layer : v->mRoi->objectGroups()) {
                    const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
                    for (const Qroilib::RoiObject *roiObject : objectGroup) {
                        pData = (RoiObject *)roiObject;
                        break;
                    }
                }
                if (pData == nullptr) {
                    ExecApplication(iplImg, iplImg2);
                    return;
                }

                IplImage* croppedImage;
                QRectF rect = pData->bounds();	// Area로 등록된 ROI.
                rect.normalized();

                if (rect.left() < 0) rect.setLeft(0);
                if (rect.top() < 0)	rect.setTop(0);
                if (rect.right() >= iplImg->width) rect.setRight(iplImg->width);
                if (rect.bottom() >= iplImg->height) rect.setBottom(iplImg->height);
                pData->setBounds(rect);

                cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
                cvSetImageROI(iplImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
                croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), iplImg->depth, iplImg->nChannels);
                cvCopy(iplImg, croppedImage);
                cvResetImageROI(iplImg);

                ExecApplication(croppedImage, iplImg2);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecApplication(iplImg, iplImg2);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            if (method == 4)
                iplImg = pView->getIplcolor();
            else
                iplImg = pView->getIplgray();

            if (!iplImg)
                return;
            ExecApplication(iplImg, iplImg2);
        }
    }
}

void DialogApplication::restoreLoadedImage()
{
    if (backImg != nullptr)
    {
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        QImage img;
        cv::Mat m = cvarrToMat(backImg);
        mat_to_qimage(m, img);
        Qroilib::DocumentView* v = pView->currentView();
        v->document()->setImageInternal(img);
        v->imageView()->updateBuffer();
    }
}

void DialogApplication::on_radioButtonCenterOfPlusMark_clicked()
{
    restoreLoadedImage();
    method = 0;
}
void DialogApplication::on_radioButtonCodeScanner_clicked()
{
    restoreLoadedImage();
    method = 1;
}
void DialogApplication::on_radioButtonGeoMatch_clicked()
{
    restoreLoadedImage();
    method = 2;
}
void DialogApplication::on_radioButtonXFeatures2D_clicked()
{
    restoreLoadedImage();
    method = 3;
}
void DialogApplication::on_radioButtonColorDetect_clicked()
{
    restoreLoadedImage();
    method = 4;
}
void DialogApplication::on_radioButtonFFT_clicked()
{
    ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
    IplImage* iplImg = pView->getIplcolor();
    if (iplImg == nullptr)
        return;
    if (backImg == nullptr)
        backImg = cvCloneImage(iplImg);

    QImage img;
    cv::Mat m = convertFFTMag();
    mat_to_qimage(m, img);

    Qroilib::DocumentView* v = pView->currentView();
    v->document()->setImageInternal(img);
    v->imageView()->updateBuffer();

    method = 5;
}

cv::Mat DialogApplication::convertFFTMag()
{
    ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
    IplImage* iplImg = pView->getIplgray();
    cv::Mat inputImage = cvarrToMat(iplImg);

    // Go float
    inputImage.convertTo(fImage, CV_32F);
    fImage = fImage(cv::Rect(0, 0, fImage.cols & -2, fImage.rows & -2));

    cv::Mat padded; //expand input image to optimal size
    int m = cv::getOptimalDFTSize( fImage.rows );
    int n = cv::getOptimalDFTSize( fImage.cols ); // on the border add zero values
    cv::copyMakeBorder(fImage, padded, 0, m - fImage.rows, 0, n - fImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

    // FFT
    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::Mat fourierTransform;
    cv::dft(fImage, fourierTransform, cv::DFT_SCALE|cv::DFT_COMPLEX_OUTPUT);

    fourierTransform.copyTo(ftI);
    ftI = ftI(cv::Rect(0, 0, ftI.cols & -2, ftI.rows & -2));
    int cx = ftI.cols/2;
    int cy = ftI.rows/2;
    cv::Mat f0(ftI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    cv::Mat f1(ftI, cv::Rect(cx, 0, cx, cy));  // Top-Right
    cv::Mat f2(ftI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
    cv::Mat f3(ftI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right
    cv::Mat tmp;                       // swap quadrants (Top-Left with Bottom-Right)
    f0.copyTo(tmp);
    f3.copyTo(f0);
    tmp.copyTo(f3);
    f1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    f2.copyTo(f1);
    tmp.copyTo(f2);

    cv::split(ftI, planes);       // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
    cv::Mat magI;
    magI = planes[0];

    magI += cv::Scalar::all(1);         // switch to logarithmic scale
    log(magI, magI);
    normalize(magI, magI, 0, 2048, cv::NORM_MINMAX); // Transform the matrix with float values into a
                         // viewable image form (float between values 0 and 1024).

    //cv::imshow("fourierTransform", magI);
    magI.convertTo(displayImage, CV_8U);

    return displayImage;
}

void DialogApplication::DrawMark(IplImage* iplImg, int x, int y)
{
    CvPoint pt1, pt2;
    pt1.x = x - 40;
    pt1.y = y;
    pt2.x = x + 40;
    pt2.y = y;
    cvLine(iplImg, pt1, pt2, CV_RGB(192, 192, 192), 1, 8, 0);
    pt1.x = x;
    pt1.y = y - 40;
    pt2.x = x;
    pt2.y = y + 40;
    cvLine(iplImg, pt1, pt2, CV_RGB(192, 192, 192), 1, 8, 0);
}

void DialogApplication::ExecApplication(IplImage* iplImg, IplImage* iplImg2)
{
    switch(method)
    {
    case 0:
        centerOfPlusmaek(iplImg);
        break;
    case 1:
        codeScanner(iplImg);
        break;
    case 2:
        ExecGeoMatch(iplImg, iplImg2);
        break;
    case 3:
        xfeatureTest(iplImg, iplImg2);
        break;
    case 4:
        ColorDetect(iplImg, iplImg2);
        break;
    case 5:
        FFTTest();
        break;
    }
    theMainWindow->outWidget(mName, outImg);
}


void DialogApplication::centerOfPlusmaek(IplImage* iplImg)
{
    if (outImg) {
        if (outImg->width != iplImg->width || outImg->height != iplImg->height) {
            cvReleaseImage(&outImg);
            outImg = nullptr;
        }
    }

    if (!outImg)
        outImg = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);
    cvCopy(iplImg, outImg);
    //cvZero(outImg);

    CImgProcBase base;

    IplImage* img2 = cvCloneImage(iplImg);
    base.FilterLargeArea(img2);

    CBlobResult blobs;
    blobs = CBlobResult(img2, NULL);
    int num = blobs.GetNumBlobs();
    if (num == 0)
        return;
    CBlob *p = blobs.GetBlob(0);
    CvRect r = p->GetBoundingBox();
    uchar* imageData;
    int cx = r.width;
    int cy = r.height;
    int imgStep;
    int tcy = (cy * 0.5) * 255;
    int tcx = (cx * 0.5) * 255;

    IplImage* croppedImage = cvCreateImage(cvSize(r.width, r.height), img2->depth, img2->nChannels);

    base.CopyImageROI(img2, croppedImage, r);
    // 좌우 Trim
    imageData = (uchar*)croppedImage->imageData;
    imgStep = croppedImage->widthStep;
    for (int i = 0; i < croppedImage->width; i++) {	// 50% 이상 black이면 검정색으로채움
        unsigned long sum = 0;
        for (int j = 0; j < croppedImage->height; j++)
            sum += imageData[imgStep * j + i];
        if (sum < tcy) {
            for (int j = 0; j < croppedImage->height; j++)
                imageData[imgStep * j + i] = 0;
        }
        else break;
    }
    for (int i = croppedImage->width-1; i >= 0; i--) {
        unsigned long sum = 0;
        for (int j = 0; j < croppedImage->height; j++)
            sum += imageData[imgStep * j + i];
        if (sum < tcy) {
            for (int j = 0; j < croppedImage->height; j++)
                imageData[imgStep * j + i] = 0;
        }
        else break;
    }

    //cvShowImage("Vertical", croppedImage);
    ExecRansacLinefit(croppedImage, 0); // 세로선
    //...

    cvSetImageROI(outImg, r);
    cvXor(croppedImage, outImg, outImg);
    cvResetImageROI(outImg);


    base.CopyImageROI(img2, croppedImage, r);
    imgStep = croppedImage->widthStep;
    imageData = (uchar*)croppedImage->imageData;
    // 상하 Trim
    for (int i = 0; i < croppedImage->height; i++) {	// 50% 이상 black이면 검정색으로채움
        unsigned long sum = 0;
        for (int j = 0; j < croppedImage->width; j++)
            sum += imageData[imgStep * i + j];
        if (sum < tcx) {
            for (int j = 0; j < croppedImage->width; j++)
                imageData[imgStep * i + j] = 0;
        }
        else break;
    }
    for (int i = croppedImage->height - 1; i >= 0; i--) {
        unsigned long sum = 0;
        for (int j = 0; j < croppedImage->width; j++)
            sum += imageData[imgStep * i + j];
        if (sum < tcx) {
            for (int j = 0; j < croppedImage->width; j++)
                imageData[imgStep * i + j] = 0;
        }
        else break;
    }

    //cvShowImage("Horizontal", croppedImage);
    ExecRansacLinefit(croppedImage, 2); // 가로선
    //...

    cvSetImageROI(outImg, r);
    cvXor(croppedImage, outImg, outImg);
    cvResetImageROI(outImg);

    Point intPnt;
    CImgProcBase img;
    img.getIntersectionPoint(plusmarkpt[0],plusmarkpt[1],plusmarkpt[2],plusmarkpt[3], intPnt);

    intPnt.x += p->MinX();
    intPnt.y += p->MinY();

    char text[128];
    sprintf(text, "%d,%d",  intPnt.x, intPnt.y);
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, CV_AA);

    cvRectangle(outImg, CvPoint(0,0), CvPoint(outImg->width,10), cvScalar(255, 255, 255), CV_FILLED);
    cvPutText(outImg, text, cvPoint(10, 10), &font, cvScalar(128, 128, 128));



    if (img2) cvReleaseImage(&img2);
    if (croppedImage) cvReleaseImage(&croppedImage);

}



void DialogApplication::ExecRansacLinefit(IplImage* iplImg, int offset)
{
    IplImage* img2 = cvCloneImage(iplImg);
    cvZero(img2);

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    cvFindContours(iplImg, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    float line[4];
    while(contours)
    {
        if (contours->total < 10)
        {
            //contours = contours->h_next;
            //continue;
        }

        // CV_DIST_L2 : 거리 유형
        // 0 : L2 거리 사용하지 않음
        // 0.01 : 정확도
        cvFitLine(contours, CV_DIST_L2, 0, 0.01, 0.01, line);
        CvRect boundbox = cvBoundingRect(contours);
        //int xlen = boundbox.width / 2;
        //int ylen = boundbox.height / 2;

        double d = sqrt(line[0] * line[0] + line[1] * line[1]);
        line[0] /= d;
        line[1] /= d;
        double t = boundbox.width + boundbox.height;

        // 올바른 선을 계산하는지 확인하기 위해 영상에 예상 선을 그림
        int x0= line[2]; // 선에 놓은 한 점
        int y0= line[3];
        int x1= x0 - t * floor(line[0]+0.5); // 기울기에 길이를 갖는 벡터 추가
        int y1= y0 - t * floor(line[1]+0.5);
        int x2= x0 + t * floor(line[0]+0.5);
        int y2= y0 + t * floor(line[1]+0.5);
        cvLine( img2, CvPoint(x1,y1), CvPoint(x2,y2), CV_RGB(128,128,128), 1, 8 );

        plusmarkpt[offset+0] = CvPoint(x1,y1);
        plusmarkpt[offset+1] = CvPoint(x2,y2);

        contours = contours->h_next;
    }

    cvReleaseMemStorage(&storage);
    cvCopy(img2, iplImg);

}

void DialogApplication::codeScanner(IplImage* iplImg)
{
    if (outImg) {
        if (outImg->width != iplImg->width || outImg->height != iplImg->height) {
            cvReleaseImage(&outImg);
            outImg = nullptr;
        }
    }

    if (!outImg)
        outImg = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);
    //cvCopy(iplImg, outImg);
    cvZero(outImg);

    QZXing qz;

    cv::Mat m = cv::cvarrToMat(iplImg);
    QImage img;
    mat_to_qimage(m, img);

    cvCopy(iplImg, outImg);

    QString decode;
    try {
        decode = qz.decodeImage(img);
    }
    catch(zxing::NotFoundException  &e){}
    catch(zxing::ReaderException  &e){}
    qDebug() << "decode " << decode;

    char text[128];
    sprintf(text, "%s", decode.toLocal8Bit().data());
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
    cvPutText(outImg, text, cvPoint(10, 16), &font, cvScalar(128, 128, 128, 0));

}


void DialogApplication::ExecGeoMatch(IplImage* iplImg, IplImage* iplImg2)
{
    if (iplImg == nullptr)
        return;
    if (iplImg2 == nullptr)
        return;
    int w = max(iplImg->width, iplImg2->width);
    int h = max(iplImg->height, iplImg2->height);

    if (outImg) {
        if (outImg->width != w || outImg->height != h) {
            cvReleaseImage(&outImg);
            outImg = nullptr;
        }
    }
    if (!outImg)
        outImg = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
    cvZero(outImg);


    CvRect rect = CvRect(0,0,iplImg2->width, iplImg2->height);
    cvSetImageROI(outImg, rect);
    cvCopy(iplImg2, outImg);
    cvResetImageROI(outImg);

    int nGaussian = 3;
    try {
        cvSmooth(iplImg2, iplImg2, CV_GAUSSIAN,nGaussian,nGaussian);
    } catch (...) {
        qDebug() << "Error cvSmooth()";
        return;
    }

    GeoMatch GM;				// object to implent geometric matching
    CvPoint result;
    double score= 0;

    int lowThreshold = ui->editGeoLow->text().toInt(); // 10
    int highThreashold = ui->editGeoHigh->text().toInt(); // 100
    float minScore = ui->editGeoScore->text().toDouble(); // 0.7
    float greediness = ui->editGeoGreediness->text().toDouble(); // 0.5

    if(!GM.CreateGeoMatchModel(iplImg,lowThreshold,highThreashold))
    {
        qDebug() <<"ERROR: could not create model...";
        return;
    }
    GM.DrawContours(outImg,CV_RGB( 128, 128, 128 ),1);
    qDebug() <<" Shape model created.."<<"with  Low Threshold = "<<lowThreshold<<" High Threshold = "<<highThreashold;


    clock_t start_time1 = clock();
    score = GM.FindGeoMatchModel(iplImg2,minScore,greediness,&result);
    clock_t finish_time1 = clock();
    double total_time = (double)(finish_time1-start_time1)/CLOCKS_PER_SEC;

    qDebug() <<" Found at ["<<result.x<<", "<<result.y<<"]\n Score = "<<score<<"\n Searching Time = "<<total_time*1000<<"ms";
    GM.DrawContours(outImg,result,CV_RGB( 128, 128, 128 ),1);


    char text[128];
    sprintf(text, "score:%.1f",  score);
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

    cvRectangle(outImg, CvPoint(0,0), CvPoint(outImg->width/2,20), cvScalar(255, 255, 255), CV_FILLED);
    cvPutText(outImg, text, cvPoint(10, 16), &font, cvScalar(128, 128, 128));


    //cv::imshow("outImg", cvarrToMat(outImg));
}

//
// 참조:
// http://webnautes.tistory.com/1152
// https://github.com/oreillymedia/Learning-OpenCV-3_examples/blob/master/example_16-02.cpp
// https://introlab.github.io/find-object/
//
int DialogApplication::xfeatureTest(IplImage* iplImg, IplImage* iplImg2)
{
    int w = max(iplImg->width, iplImg2->width);
    int h = max(iplImg->height, iplImg2->height);

    if (outImg) {
        if (outImg->width != w || outImg->height != h) {
            cvReleaseImage(&outImg);
            outImg = nullptr;
        }
    }
    if (!outImg)
        outImg = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
    cvZero(outImg);


    CvRect rect = CvRect(0,0,iplImg2->width, iplImg2->height);
    cvSetImageROI(outImg, rect);
    cvCopy(iplImg2, outImg);
    cvResetImageROI(outImg);


    int nGaussian = 3;
    try {
        cvSmooth(iplImg2, iplImg2, CV_GAUSSIAN,nGaussian,nGaussian);
    } catch (...) {
        qDebug() << "Error cvSmooth()";
        return -1;
    }

    int nParam1 = ui->editParam1->text().toInt();

    QString detectorName;
    detectorName = ui->comboBoxDetector->currentText(); // "SURF";
    std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
    cv::Mat descriptors_object, descriptors_scene;
    vector<vector<cv::DMatch>> m_knnMatches;
    std::vector< cv::DMatch > good_matches;

    cv::Mat img_object = cv::cvarrToMat(iplImg);
    cv::Mat img_scene =  cv::cvarrToMat(iplImg2);

    if (detectorName == "SIFT") { // distance:10, MaxSize:200
        SIFTDetector sift(nParam1); //  nParam1=0
        sift(img_object, cv::Mat(), keypoints_object, descriptors_object);
        sift(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher;
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else if (detectorName == "SURF") { // distance:5, MaxSize:200
        SURFDetector surf((double)nParam1); //  nParam1=800
        surf(img_object, cv::Mat(), keypoints_object, descriptors_object);
        surf(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher;
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else if (detectorName == "ORB") { // distance:5, MaxSize:200
        ORBDetector orb(nParam1); // nParam1=2000
        orb(img_object, cv::Mat(), keypoints_object, descriptors_object);
        orb(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher(cv::NORM_HAMMING); // use cv::NORM_HAMMING2 for ORB descriptor with WTA_K == 3 or 4 (see ORB constructor)
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else return -1;

    for(int i = 0; i < min(img_scene.rows-1,(int) m_knnMatches.size()); i++) //THIS LOOP IS SENSITIVE TO SEGFAULTS
    {
        const cv::DMatch& bestMatch = m_knnMatches[i][0];
        if((int)m_knnMatches[i].size()<=2 && (int)m_knnMatches[i].size()>0)
        {
            good_matches.push_back(bestMatch);
        }
    }

    if (good_matches.size() == 0)
        return -1;

    double dDist = ui->editDistanceCoef->text().toDouble();
    int nMax = ui->editMaxMatchingSize->text().toInt();

    const double kDistanceCoef = dDist;//4.0;
    const int kMaxMatchingSize = nMax;//50;

    std::sort(good_matches.begin(), good_matches.end());
    while (good_matches.front().distance * kDistanceCoef < good_matches.back().distance) {
        good_matches.pop_back();
    }
    while (good_matches.size() > kMaxMatchingSize) {
        good_matches.pop_back();
    }

    std::vector<cv::Point2f> corner;
    cv::Mat img_matches = drawGoodMatches(img_object, img_scene, keypoints_object, keypoints_scene, good_matches, corner);

    //-- Show detected matches
    if (img_matches.rows > 0)
    {
        cv::namedWindow("knnMatch", 0);
        cv::imshow("knnMatch", img_matches);
    }

    if (corner.size() > 0)
    {
        cv::Point pt1 = corner[0];
        cv::Point pt2 = corner[1];
        cv::Point pt3 = corner[2];
        cv::Point pt4 = corner[3];
        cvLine(outImg, pt1, pt2, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
        cvLine(outImg, pt2, pt3, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
        cvLine(outImg, pt3, pt4, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
        cvLine(outImg, pt4, pt1, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);

        cvRectangle(iplImg2, pt1, pt3, cvScalar(255, 255, 255), CV_FILLED); // test
        theMainWindow->outWidget("test1", iplImg2);

    }

    return 0;
}

cv::Mat DialogApplication::drawGoodMatches(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& keypoints1,
    const std::vector<cv::KeyPoint>& keypoints2,
    std::vector<cv::DMatch>& good_matches,
    std::vector<cv::Point2f>& scene_corners_
)
{
    cv::Mat img_matches;
    if (good_matches.size() == 0)
        return img_matches;

    cv::drawMatches(img1, keypoints1, img2, keypoints2,
        good_matches, img_matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
        std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    //-- Localize the object
    std::vector<cv::Point2f> obj;
    std::vector<cv::Point2f> scene;

    for (size_t i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
    }
    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<cv::Point2f> obj_corners(4);
    obj_corners[0] = cv::Point(0, 0);
    obj_corners[1] = cv::Point(img1.cols, 0);
    obj_corners[2] = cv::Point(img1.cols, img1.rows);
    obj_corners[3] = cv::Point(0, img1.rows);
    std::vector<cv::Point2f> scene_corners(4);

    cv::Mat H;
    try {
        H = cv::findHomography(obj, scene, cv::RANSAC);
        perspectiveTransform(obj_corners, scene_corners, H);
    } catch (...) {
        qDebug() << ("Error <unknown> findHomography()");
        return img_matches;
    }

    scene_corners_ = scene_corners;

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    cv::Point pt1 = scene_corners[0] + cv::Point2f((float)img1.cols, 0);
    cv::Point pt2 = scene_corners[1] + cv::Point2f((float)img1.cols, 0);
    cv::Point pt3 = scene_corners[2] + cv::Point2f((float)img1.cols, 0);
    cv::Point pt4 = scene_corners[3] + cv::Point2f((float)img1.cols, 0);
    line(img_matches, pt1, pt2, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
    line(img_matches, pt2, pt3, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
    line(img_matches, pt3, pt4, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
    line(img_matches, pt4, pt1, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);

    return img_matches;
}

void DialogApplication::ColorDetect(IplImage* iplImg, IplImage* iplImg2)
{
    if (outImg) {
        if (outImg->width != iplImg->width || outImg->height != iplImg->height) {
            cvReleaseImage(&outImg);
            outImg = nullptr;
        }
    }

    if (!outImg)
        outImg = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, 1);
    //cvCopy(iplImg, outImg);
    cvZero(outImg);

    int range_count = 0;

#if 0
    Scalar red(0, 0, 255);
    Scalar blue(255, 0, 0);
    Scalar yellow(0, 255, 255);

    Scalar magenta(255, 0, 255);


    Mat rgb_color = Mat(1, 1, CV_8UC3, red);
    Mat hsv_color;

    cvtColor(rgb_color, hsv_color, COLOR_BGR2HSV);


    int hue = (int)hsv_color.at<Vec3b>(0, 0)[0];
    int saturation = (int)hsv_color.at<Vec3b>(0, 0)[1];
    int value = (int)hsv_color.at<Vec3b>(0, 0)[2];


    cout << "hue = " << hue << endl;
    cout << "saturation = " << saturation << endl;
    cout << "value = " << value << endl;
#endif

    int low_hue = ui->editHue1->text().toInt();
    int high_hue = ui->editHue2->text().toInt();
    int low_saturation = ui->editSaturation1->text().toInt();
    int high_saturation = ui->editSaturation2->text().toInt();

//    int low_hue = hue - 10;
//    int high_hue = hue + 10;

    int low_hue1 = 0, low_hue2 = 0;
    int high_hue1 = 0, high_hue2 = 0;

    if (low_hue > high_hue ) {
        range_count = 2;

        high_hue1 = 180;
        low_hue1 = low_hue;
        high_hue2 = high_hue;
        low_hue2 = 0;
    }
    else {
        range_count = 1;

        low_hue1 = low_hue;
        high_hue1 = high_hue;
    }

    cout << low_hue1 << "  " << high_hue1 << endl;
    cout << low_hue2 << "  " << high_hue2 << endl;



    //VideoCapture cap("test.mp4");

    Mat img_hsv;

    //for (;;)
    {
        // wait for a new frame from camera and store it into 'frame'
        //cap.read(img_frame);
        cv::Mat img_frame = cv::cvarrToMat(iplImg);

        // check if we succeeded
        if (img_frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            return;
        }


        //HSV로 변환
        cvtColor(img_frame, img_hsv, COLOR_BGR2HSV);


        //지정한 HSV 범위를 이용하여 영상을 이진화
        Mat img_mask1, img_mask2;
        inRange(img_hsv, Scalar(low_hue1, low_saturation, 50), Scalar(high_hue1, high_saturation, 255), img_mask1);
        if (range_count == 2) {
            inRange(img_hsv, Scalar(low_hue2, low_saturation, 50), Scalar(high_hue2, high_saturation, 255), img_mask2);
            img_mask1 |= img_mask2;
        }


        //morphological opening 작은 점들을 제거
        erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));


        //morphological closing 영역의 구멍 메우기
        dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        //morphological opening 작은 점들을 제거
        erode(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));
        dilate(img_mask1, img_mask1, getStructuringElement(MORPH_ELLIPSE, Size(8, 8)));


        //라벨링
        Mat img_labels, stats, centroids;
        int numOfLables = connectedComponentsWithStats(img_mask1, img_labels,
            stats, centroids, 8, CV_32S);


        //영역박스 그리기
        int max = -1, idx = 0;
        for (int j = 1; j < numOfLables; j++) {
            int area = stats.at<int>(j, CC_STAT_AREA);
            if (max < area)
            {
                max = area;
                idx = j;
            }
        }


        int left = stats.at<int>(idx, CC_STAT_LEFT);
        int top = stats.at<int>(idx, CC_STAT_TOP);
        int width = stats.at<int>(idx, CC_STAT_WIDTH);
        int height = stats.at<int>(idx, CC_STAT_HEIGHT);


        Mat channels[3];
        split(img_hsv, channels);
        img_mask1 = channels[0] | img_mask1;


        rectangle(img_mask1, Point(left, top), Point(left + width, top + height),
            Scalar(128, 128, 128), 2);


        IplImage ipl = img_mask1;
        cvCopy(&ipl, outImg);

        //imshow("이진화 영상", img_mask1);
        //imshow("원본 영상", img_frame);


        //if (waitKey(5) >= 0)
        //    break;
    }


    return;
}


void DialogApplication::FFTTest()
{
    cv::Mat fourierTransform;

    int cols2 = ftI.cols;
    int rows2 = ftI.rows;
    ftI.copyTo(fourierTransform);

    cv::Mat padded; //expand input image to optimal size
    int m = cv::getOptimalDFTSize( fImage.rows );
    int n = cv::getOptimalDFTSize( fImage.cols ); // on the border add zero values
    cv::copyMakeBorder(fImage, padded, 0, m - fImage.rows, 0, n - fImage.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::Mat planes[] = {cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F)};
    cv::split(fourierTransform, planes);       // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    float *pdata1 = (float *)planes[0].data;
    float *pdata2 = (float *)planes[1].data;

    ViewMainPage *viewMain = (ViewMainPage *)theMainWindow->viewMainPage();
    Qroilib::DocumentView* v = viewMain->currentView();
    if (v != nullptr) {
        RoiObject *pData = nullptr;
        for (const Layer *layer : v->mRoi->objectGroups()) {
            const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
            for (const Qroilib::RoiObject *roiObject : objectGroup) {
                pData = (RoiObject *)roiObject;

                QRectF rect = pData->bounds();	// Area로 등록된 ROI.
                const int szx = rect.width();
                const int szy = rect.height();
                for (int y = rect.y(); y < rect.y()+szy; y++) {
                    for (int x = rect.x(); x < rect.x()+szx; x++) {
                        pdata1[x + (cols2*y)] = 0;
                        pdata2[x + (cols2*y)] = 0;
                    }
                }
            }
        }
    }

    cv::merge(planes, 2, fourierTransform);

    int cx2 = fourierTransform.cols/2;
    int cy2 = fourierTransform.rows/2;
    cv::Mat x0(fourierTransform, cv::Rect(0, 0, cx2, cy2));   // Top-Left - Create a ROI per quadrant
    cv::Mat x1(fourierTransform, cv::Rect(cx2, 0, cx2, cy2));  // Top-Right
    cv::Mat x2(fourierTransform, cv::Rect(0, cy2, cx2, cy2));  // Bottom-Left
    cv::Mat x3(fourierTransform, cv::Rect(cx2, cy2, cx2, cy2)); // Bottom-Right
    cv::Mat tmp2;                       // swap quadrants (Top-Left with Bottom-Right)
    x0.copyTo(tmp2);
    x3.copyTo(x0);
    tmp2.copyTo(x3);
    x1.copyTo(tmp2);                    // swap quadrant (Top-Right with Bottom-Left)
    x2.copyTo(x1);
    tmp2.copyTo(x2);

    // IFFT
    //std::cout << "Inverse transform...\n";
    cv::Mat inverseTransform;
    cv::dft(fourierTransform, inverseTransform, cv::DFT_INVERSE|cv::DFT_REAL_OUTPUT);

    // Back to 8-bits
    cv::Mat finalImage;
    inverseTransform.convertTo(finalImage, CV_8U);
    //cv::imshow("inverseTransform", finalImage);

    if (outImg) {
        if (outImg->width != finalImage.cols || outImg->height != finalImage.rows) {
            cvReleaseImage(&outImg);
            outImg = nullptr;
        }
    }

    if (!outImg)
        outImg = cvCreateImage(cvSize(finalImage.cols, finalImage.rows), IPL_DEPTH_8U, 1);


    IplImage ipl = finalImage;
    cvCopy(&ipl, outImg);
}
