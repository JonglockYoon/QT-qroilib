/*

QROILIB : QT Vision ROI Library
Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

이 Program은 Machine Vision Program을 QT 환경에서 쉽게 구현할수 있게 하기 위해 작성되었다.

Gwenview 의 Multi view 기능과,
Tiled의 Object drawing 기능을 가져와서 camera의 이미지를 실시간 display하면서 vision ROI를 작성하여,
내가 원하는 결과를 OpenCV를 통하여 쉽게 구현할수 있도록 한다.

이 Program은 ARM계열 linux  와 X86계열 linux 및 windows에서 test되었다.

------

qroieditor:
    ROI editor sample program

*/

#include <roilib_export.h>
// Qt
#include <QApplication>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>
#include <QMenuBar>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>

// Local
#include <qroilib/gvdebug.h>
#include "mainwindow.h"
#include "common.h"
#include "recipedata.h"

struct MainWindow::Private
{
    MainWindow* q;
    QWidget* mContentWidget;
    ViewMainPage* mViewMainPage;
    QStackedWidget* mViewStackedWidget;
    QActionGroup* mViewModeActionGroup;

    QMenu *fileMenu;
    QMenu *viewMenu;

    QAction *exitAct;

    void setupWidgets()
    {
        mContentWidget = new QWidget(q);
        q->setCentralWidget(mContentWidget);

        mViewStackedWidget = new QStackedWidget(mContentWidget);
        QVBoxLayout* layout = new QVBoxLayout(mContentWidget);
        layout->addWidget(mViewStackedWidget);
        layout->setMargin(0);
        layout->setSpacing(0);

        setupViewMainPage(mViewStackedWidget);
        mViewStackedWidget->addWidget(mViewMainPage);

    }

    void setupViewMainPage(QWidget* parent)
    {
        mViewMainPage = new ViewMainPage(parent);
    }

    void setupActions()
    {
        exitAct = new QAction(QIcon(), tr("E&xit"), q);
        exitAct->setShortcuts(QKeySequence::Quit);
        connect(exitAct, SIGNAL(triggered()), q, SLOT(close()));

        fileMenu = new QMenu(tr("&File"), q);

        fileMenu->addAction(exitAct);
        viewMenu = new QMenu(tr("&View"), q);

        q->menuBar()->addMenu(fileMenu);
        q->menuBar()->addMenu(viewMenu);

    }

};

MainWindow* theMainWindow = 0;

MainWindow::MainWindow()
: d(new MainWindow::Private)
{
    theMainWindow = this;

    setMinimumSize(800, 480);
    g_cRecipeData = new CRecipeData(this);

    d->q = this;
    d->setupWidgets();
    d->setupActions();
    d->mViewMainPage->loadConfig();

    extern Qroilib::ParamTable ROIDSHARED_EXPORT paramTable[];
    DocumentView* v = currentView();
    if (v)
    {
        v->setParamTable(paramTable);
        v->bMultiView = false;
        connect(v, &Qroilib::DocumentView::finishNewRoiObject, this, &MainWindow::finishNewRoiObject);
        v->selectTool(v->actSelectTool);
    }

    mLeftDock = new LeftDock(tr("Menu"), this);
    mLeftDock->setFeatures(QDockWidget::DockWidgetClosable);
    addDockWidget(Qt::LeftDockWidgetArea, mLeftDock);
    d->viewMenu->addAction(mLeftDock->toggleViewAction());

}


MainWindow::~MainWindow()
{
}

ViewMainPage* MainWindow::viewMainPage() const
{
    return d->mViewMainPage;
}

DocumentView* MainWindow::currentView() const
{
    DocumentView* v = d->mViewMainPage->currentView();
    return v;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

    event->ignore();
    QString str = tr("Close application");
    qDebug() << str;

    int ret = QMessageBox::warning(this, str,
                                   tr("Do you really want to quit?"),
                                   QMessageBox::Ok| QMessageBox::Cancel,
                                   QMessageBox::Ok);
    switch(ret)
    {
        case QMessageBox::Ok:
            event->accept();

            delete d->mViewMainPage;
            delete d;

            qApp->quit(); // terminates application.
            break;
    }
}

void MainWindow::finishNewRoiObject()
{
    Qroilib::DocumentView* v = currentView();
    v->clearSelectedObjectItems();
    v->selectTool(v->actSelectTool);
}

int MainWindow::InspectOneItem(IplImage* img, RoiObject *pData)
{

    QString str;
    str.sprintf("InspectOneItem type=%d", pData->mInspectType);

    if (pData == nullptr)
        return -1;

    {
        str.sprintf("0_%d_srcImage.jpg", 100);
        SaveOutImage(img, pData, str, false);
    }

    int size = pData->m_vecDetectResult.size();
    for (int i = 0; i < size; i++) {
        DetectResult *prst = &pData->m_vecDetectResult[i];
        if (prst->img)
            cvReleaseImage(&prst->img);
    }
    pData->m_vecDetectResult.clear();

    CvSize searchSize = cvSize(img->width, img->height);
    IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);

    if (img->nChannels == 3)
        cvCvtColor(img, graySearchImg, CV_RGB2GRAY);
    else if (img->nChannels == 4) {
        if (strncmp(img->channelSeq, "BGRA", 4) == 0)
            cvCvtColor(img, graySearchImg, CV_BGRA2GRAY);
        else
            cvCvtColor(img, graySearchImg, CV_RGBA2GRAY);
    } else
        cvCopy(img, graySearchImg);

    IplImage* croppedImage;
    QRectF rect = pData->bounds();	// Area로 등록된 ROI.
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= graySearchImg->width) rect.setRight(graySearchImg->width);
    if (rect.bottom() >= graySearchImg->height) rect.setBottom(graySearchImg->height);
    pData->setBounds(rect);

    cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
    cvSetImageROI(graySearchImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
    croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), graySearchImg->depth, graySearchImg->nChannels);
    cvCopy(graySearchImg, croppedImage);
    cvResetImageROI(graySearchImg);

    switch (pData->mInspectType)
    {
    case _Inspect_Patt_Identify:
        {
        double LimitMatchRate = 60.0;
        if (pData != nullptr)
        {
            CParam *pParam = pData->getParam(("Pattern matching rate"));
            if (pParam)
                LimitMatchRate = pParam->Value.toDouble();
        }
        double max = SinglePattFind(croppedImage, pData, rect);
        if ((max*100.0) > LimitMatchRate)
        {
            DetectResult *prst = &pData->m_vecDetectResult[0];
            // 찾은 물체에 사각형 박스를 그린다.
            cvRectangle(img, CvPoint(prst->tl.x,prst->tl.y),
                        CvPoint(prst->br.x,prst->br.y), CV_RGB(255,0,0), 3);
        }
        }
        break;
    case _Inspect_Roi_Find_Shape:
        {
        SingleROIFindShape(croppedImage, pData, rect);

        DrawResultCrossMark(img, pData);
        int size = pData->m_vecDetectResult.size();
        if (size > 0) {
            DetectResult *prst = &pData->m_vecDetectResult[0];
            const QString stype[] = { "triangle", "Parallelogram", "rectangle", "Rhombus","Square","pentagon","hexagon","circle","unknown" };
            int t = (int)prst->dValue;
            if (t < 0) t = 8;
            str = QString("type : %1(%2)").arg(t).arg(stype[t]);
            CvFont font;
            cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 3, CV_AA);
            cvPutText(img, str.toLatin1().data(), cvPoint(40, 40), &font, cvScalar(255, 255, 126, 0));

        }
        }
        break;
    }

    cvReleaseImage(&croppedImage);
    cvReleaseImage(&graySearchImg);

    viewMainPage()->bPreview = false;
}


//
// 모폴리지를 이용하여 잡음제거.
//
int MainWindow::NoiseOut(RoiObject *pData, IplImage* grayImg, int t, int nDbg, int h)
{
    QString str;

    if (t < 0)
        t = _ProcessValue1;

    // 1. Template이미지의 노이즈 제거.
    int filterSize = 3;  // 필터의 크기를 6으로 설정 (Noise out area)
    IplConvKernel *element = nullptr;
    if (filterSize <= 0)
        filterSize = 1;
    if (filterSize % 2 == 0)
        filterSize++;
    element = cvCreateStructuringElementEx(filterSize, filterSize, filterSize / 2, filterSize / 2, CV_SHAPE_RECT, nullptr);

    int nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 1"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else //if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 2"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else //if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 3"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_OPEN, -nNoiseout);
        else //if (nNoiseout > 0)
            cvMorphologyEx(grayImg, grayImg, nullptr, element, CV_MOP_CLOSE, nNoiseout);
    }

    {
        if (h >= 0)
            str.sprintf(("%d_%03d_cvClose.jpg"), h, nDbg);
        else str.sprintf(("%03d_cvClose.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str, false);
    }

    cvReleaseStructuringElement(&element);
    return 0;
}


int MainWindow::SingleROIFindShape(IplImage* croppedImage, RoiObject *pData, QRectF rectIn)
{
    DetectResult detectResult;
    Q_UNUSED(rectIn);
    QString str;
    int retry = 0;
    CParam *pParam;
    IplImage* grayImg = nullptr;
    int nThresholdLowValue;
    int nThresholdHighValue;
    int nMinArea;

    pData->m_vecDetectResult.clear();
    if (grayImg != nullptr)
        cvReleaseImage(&grayImg);
    grayImg = cvCloneImage(croppedImage);

    nThresholdLowValue = 0;
    pParam = pData->getParam(("Low Threshold"), _ProcessValue1+retry);
    if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();

    nThresholdHighValue = 255;
    pParam = pData->getParam("High Threshold", _ProcessValue1+retry);
    if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

    double dEPS = 0.01;
    nMinArea = 0;
    pParam = pData->getParam("Minimum area");
    if (pParam)
        nMinArea = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("EPS"));
    if (pParam)
        dEPS = (double)pParam->Value.toDouble();

    {
        str.sprintf(("%d_Src.jpg"), 200);
        SaveOutImage(grayImg, pData, str);
    }

    cvInRangeS(grayImg, cv::Scalar(nThresholdLowValue), cv::Scalar(nThresholdHighValue), grayImg);

    {
        str.sprintf(("%d_Threshold.jpg"), 203);
        SaveOutImage(grayImg, pData, str);
    }

    NoiseOut(pData, grayImg, -1, 212);

    CvMemStorage *storage = cvCreateMemStorage(0); //storage area for all contours 모든 형상들을 위한 저장공간.
    CvSeq* contours = 0;         // 경계 계수를 저장할 변수.
    CvSeq* result;   //hold sequence of points of a contour 형상의 포인터의 시퀀스 잡기.

    cvFindContours(grayImg, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    //iterating through each contour 각형상별로 반복.
    while(contours)
    {
        //obtain a sequence of points of contour, pointed by the variable 'contour' 형상의 점들의 시퀀스 가져오기, 인자 ‘contour’에 의해 지정된.
        result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*dEPS, 0);
        double area = cvContourArea(contours);
        if (area > nMinArea)  //면적이 일정크기 이상이어야 한다.
        {
            {
                IplImage* drawImage = cvCreateImage(cvSize(grayImg->width, grayImg->height), grayImg->depth, 3);
                cvZero(drawImage);
                cvDrawContours(drawImage, result, CVX_WHITE, CVX_WHITE, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트.
                str.sprintf(("220_cvApproxPoly.jpg"));
                SaveOutImage(drawImage, pData, str, false);
                if (drawImage) cvReleaseImage(&drawImage);
            }

            vector<cv::Point> approx;
            int size = result->total;
            CvSeq* c = result;
            //for (CvSeq* c = result; c != nullptr; c = c->h_next)  // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
            for (int i = 0; i < size; i++)
            {
                // CvSeq로부터좌표를얻어낸다.
                CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);
                approx.push_back(*p);
            }
            //모든 코너의 각도를 구한다.
            vector<int> angle;
            //cout << "===" << size << endl;
            for (int k = 0; k < size; k++) {
                int ang = base.GetAngleABC(approx[k], approx[(k + 1) % size], approx[(k + 2)%size]);
                //cout << k<< k+1<< k+2<<"@@"<< ang << endl;
                angle.push_back(ang);
            }

            std::sort(angle.begin(), angle.end());
            int minAngle = angle.front();
            int maxAngle = angle.back();
            int threshold = 18;
            int type = -1;
            //도형을 판정한다.
            if ( size == 3 ) // triangle
                type = 0;
            else if ( size == 4 && maxAngle < 120 ) // && minAngle >= 90- threshold && maxAngle <= 90+ threshold) // rectangle
            {
                IplImage* img = cvCreateImage(cvSize(20, 20), IPL_DEPTH_8U, 1);
                cvZero(img);
                cvRectangle(img, cvPoint(5, 5), cvPoint(15,15), CV_RGB(255, 225, 255), CV_FILLED); // filled rectangle.

                CvMemStorage *s2 = cvCreateMemStorage(0); //storage area for all contours 모든 형상들을 위한 저장공간.
                CvSeq* c2 = 0;         // 경계 계수를 저장할 변수.

                cvFindContours(img, s2, &c2, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
                double matching = cvMatchShapes(c2, c, CV_CONTOURS_MATCH_I2); // 작은 값 일수록 두 이미지의 형상이 가까운 (0은 같은 모양)라는 것이된다.
                if (matching> 0.5) {
                    type = 1;
                   str = "Parallelogram";
                }
                else if (0.3 < matching && matching < 0.5) {
                    type = 2;
                   str = "rectangle";
                }
                else if (0.02 < matching && matching < 0.3) {
                    type = 3;
                   str = "Rhombus";
                }
               else {
                    type = 4;
                   str = "Square";
                }
               qDebug() << str;

               if (img) cvReleaseImage(&img);
               if (c2) cvClearSeq(c2);
               if (s2) cvReleaseMemStorage(&s2);

            } else if (size == 5 && minAngle >= 108- threshold && maxAngle <= 108+ threshold) // pentagon
            {
               str = "pentagon";
               qDebug() << str;
               type = 5;
            } else if (size == 6 && minAngle >= 120 - threshold && maxAngle <= 140 + threshold) // hexagon
            {
               str = "hexagon";
               qDebug() << str;
               type = 6;
            } else {
                // if found convex area is ~ PI*r² then it is probably a circle
               float radius;
               CvPoint2D32f center;
               cvMinEnclosingCircle(c, &center, &radius);
               double tmp = 1.0 - (area / (PI * pow(radius, 2)));
               if (tmp < 0.3) {
                   str = "circle";
                   qDebug() << str;
                   type = 7;
               }
            }

            {
                CvSeq* ptseq = cvCreateSeq((CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_POINT | CV_SEQ_FLAG_CLOSED) | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), storage);
                for (int i = 0; i < size; ++i)
                {
                    CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);
                    cvSeqPush(ptseq, p);
                }
                cv::Point2f centerPoint = base.CenterOfMoment(ptseq);
                cvClearSeq(ptseq);

                detectResult.dValue = (double)type; // shape type
                detectResult.pt = centerPoint;
                pData->m_vecDetectResult.push_back(detectResult);
                break;
            }
        }
        //obtain the next contour 다음 형상 가져오기.
        contours = contours->h_next;
    }

    if (contours) cvClearSeq(contours);
    if (storage) cvReleaseMemStorage(&storage);


    if (grayImg) cvReleaseImage(&grayImg);

    return 0;
}


double MainWindow::SinglePattFind(IplImage* grayImage, RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    if (pData->iplTemplate == nullptr)
        return -1;
    QString str;

    CvSize searchSize = cvSize(grayImage->width, grayImage->height);
    IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);
    if (grayImage->nChannels == 3)
        cvCvtColor(grayImage, graySearchImg, CV_RGB2GRAY);
    else
        cvCopy(grayImage, graySearchImg);


    CvSize templateSize = cvSize(pData->iplTemplate->width, pData->iplTemplate->height);
    IplImage* grayTemplateImg = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);
    if (pData->iplTemplate->nChannels == 3)
        cvCvtColor(pData->iplTemplate, grayTemplateImg, CV_RGB2GRAY);
    else
        cvCopy(pData->iplTemplate, grayTemplateImg);

    {
        QString str; str.sprintf(("%d_TemplateSrc0.jpg"), 140);
        SaveOutImage(pData->iplTemplate, pData, str);
    }

    double min, max;
    CvPoint left_top;
    // 상관계수를 구할 이미지.
    IplImage *coeff = cvCreateImage( cvSize( grayImage->width - grayTemplateImg->width+1,
                             grayImage->height - grayTemplateImg->height+1 ), IPL_DEPTH_32F, 1 );
    // 상관계수를 구하여 coeff에 그려준다.
    cvMatchTemplate(grayImage, grayTemplateImg, coeff, CV_TM_CCOEFF_NORMED);
    // 상관계수가 최대값을 가지는 위치를 찾는다.
    cvMinMaxLoc(coeff, &min, &max, NULL, &left_top);

    cvReleaseImage(&coeff);
    DetectResult detectResult;
    detectResult.tl = CvPoint2D32f(pData->bounds().x() + left_top.x,
                                   pData->bounds().y() + left_top.y);
    detectResult.br = CvPoint2D32f(detectResult.tl.x + pData->iplTemplate->width,
                                   detectResult.tl.y + pData->iplTemplate->height);
    pData->m_vecDetectResult.push_back(detectResult);

    return max;
}

void MainWindow::DrawResultCrossMark(IplImage *iplImage, RoiObject *pData)
{
    if (iplImage == nullptr) return;

    QRectF rect = pData->bounds();	// Area로 등록된 ROI.
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= iplImage->width) rect.setRight(iplImage->width);
    if (rect.bottom() >= iplImage->height) rect.setBottom(iplImage->height);

    int size = pData->m_vecDetectResult.size();
    for (int i = 0; i < size; i++) {
        DetectResult *prst = &pData->m_vecDetectResult[i];
        qDebug() << "DrawResultCrossMark" << prst->pt.x << prst->pt.y;

        double x = prst->pt.x + rect.x();
        double y = prst->pt.y + rect.y();

        CvPoint pt1, pt2;
        pt1.x = x - 40;
        pt1.y = y;
        pt2.x = x + 40;
        pt2.y = y;
        cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
        pt1.x = x;
        pt1.y = y - 40;
        pt2.x = x;
        pt2.y = y + 40;
        cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
    }
}


void MainWindow::SaveOutImage(IplImage* pImgOut, RoiObject *pData, QString strMsg, bool bClear/*=false*/)
{
    QString str = ("");
    if (pData != nullptr)
        str.sprintf("./[%s]%s_%s", pData->groupName().toStdString().c_str(), pData->name().toStdString().c_str(), strMsg.toStdString().c_str());
    else
        str.sprintf("./%s", strMsg.toStdString().c_str());
    cvSaveImage((const char *)str.toStdString().c_str(), pImgOut);
    if (bClear) cvZero(pImgOut);
}
