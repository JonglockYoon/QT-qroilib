#include "dialogcontour.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogcontour.h"
#include "opencv2/shape.hpp"

using namespace std;
//using namespace cv;

DialogContour::DialogContour(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogContour)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("Contour%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    ui->setupUi(this);

    source0 = 0;
    source1 = 0;
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

    ui->radioButtonDrawContour->setChecked(true);

    connect(ui->comboBoxSource0, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource0(int)));
    connect(ui->comboBoxSource1, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource1(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));

    bApproxPoly = false;
    dEPS = 0.01;
    dMinArea = 0;
    QObject::connect(ui->chkBoxApproxPoly, SIGNAL(clicked(bool)), this, SLOT(changeApproxPoly(bool)));
    QObject::connect(ui->editEPS, SIGNAL(textChanged(const QString &)), this, SLOT(setEditEPS(const QString &)));
    QObject::connect(ui->editMinArea, SIGNAL(textChanged(const QString &)), this, SLOT(setEditMinArea(const QString &)));
    str = QString("%1").arg(dEPS);
    ui->editEPS->setText(str);
    str = QString("%1").arg(dMinArea);
    ui->editMinArea->setText(str);


}

DialogContour::~DialogContour()
{
}

void DialogContour::closeEvent(QCloseEvent *event)
{
//    if (source0 == 0)
//    {
//        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
//        if (pView) {
//            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
//        }
//    } else {
//        OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
//        ViewOutPage* pView = pWidget->mViewOutPage;
//        if (pView) {
//            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
//        }
//    }

    if (outImg)
        cvReleaseImage(&outImg);
    outImg = nullptr;
}

void DialogContour::changeRealtime(bool checked)
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
void DialogContour::activatedComboBoxSource0(int act)
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
void DialogContour::activatedComboBoxSource1(int act)
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

void DialogContour::on_pushButtonClose_clicked()
{
    if (outImg)
        cvReleaseImage(&outImg);
    outImg = nullptr;
    hide();
}

void DialogContour::changeApproxPoly(bool checked)
{
    bApproxPoly = checked;
}

void DialogContour::setEditEPS(const QString &val)
{
    dEPS = val.toDouble();
}
void DialogContour::setEditMinArea(const QString &val)
{
    dMinArea = val.toDouble();
}

//
// bRealtime 처리..
//
void DialogContour::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogContour::on_pushButtonExec_clicked()
{
    IplImage* iplImg2 = nullptr;
    if (theMainWindow->vecOutWidget.size() > 0)
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
                    ExecContour(iplImg, iplImg2);
                    return;
                }

                IplImage* croppedImage;
                IplImage* croppedImage2;
                QRectF rect = pData->bounds();	// Area로 등록된 ROI.
                rect.normalized();

                if (rect.left() < 0)	rect.setLeft(0);
                if (rect.top() < 0)	rect.setTop(0);
                if (rect.right() >= iplImg->width) rect.setRight(iplImg->width);
                if (rect.bottom() >= iplImg->height) rect.setBottom(iplImg->height);
                pData->setBounds(rect);

                cv::Point2f left_top = cv::Point2f(rect.left(), rect.top());
                cvSetImageROI(iplImg, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
                croppedImage = cvCreateImage(cvSize(rect.width(), rect.height()), iplImg->depth, iplImg->nChannels);
                cvCopy(iplImg, croppedImage);
                cvResetImageROI(iplImg);

                if (iplImg2 != nullptr)
                {
                    cvSetImageROI(iplImg2, cvRect((int)left_top.x, (int)left_top.y, rect.width(), rect.height()));
                    croppedImage2 = cvCreateImage(cvSize(rect.width(), rect.height()), iplImg2->depth, iplImg2->nChannels);
                    cvCopy(iplImg2, croppedImage2);
                    cvResetImageROI(iplImg2);
                }
                ExecContour(croppedImage, croppedImage2);

                cvReleaseImage(&croppedImage);
                if (iplImg2 != nullptr)
                    cvReleaseImage(&croppedImage2);
            }
            else
                ExecContour(iplImg, iplImg2);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source0-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecContour(iplImg, iplImg2);
        }
    }
}

void DialogContour::DrawResultCrossMark(IplImage *iplImage, cv::Point2f center)
{
    if (iplImage == NULL) return;

    double x = center.x;// / gCfg.m_pCamInfo[0].dResX;
    double y = center.y;// / gCfg.m_pCamInfo[0].dResY;

    CvPoint pt1, pt2;
    pt1.x = x - 40;
    pt1.y = y;
    pt2.x = x + 40;
    pt2.y = y;
    cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 1, 8, 0);
    pt1.x = x;
    pt1.y = y - 40;
    pt2.x = x;
    pt2.y = y + 40;
    cvLine(iplImage, pt1, pt2, CV_RGB(192, 192, 192), 1, 8, 0);

}


void DialogContour::on_radioButtonDrawContour_clicked()
{
    method = 0;
    bContourRect = 0;
}
void DialogContour::on_radioButtonMatchShapes_clicked()
{
    method = 1;
}
void DialogContour::on_radioButtonShapeContextDistanceExtractor_clicked()
{
    method = 2;
}
void DialogContour::on_radioButtonDrawContourRect_clicked()
{
    method = 0;
    bContourRect = 1;
}

void DialogContour::ExecContour(IplImage* iplImg, IplImage* iplImg2)
{
    switch (method) {
    case 0: // DrawContour
        DrawContour(iplImg);
        break;
    case 1: // MatchShapes
        if(iplImg2 == nullptr)
            return;
        TestMatchShapes(iplImg, iplImg2);
        break;
    case 2: // MatchShapes
        if(iplImg2 == nullptr)
            return;
        TestShapeContextDistanceExtractor(iplImg, iplImg2);
        break;
    }

    theMainWindow->outWidget(mName, outImg);
}


void DialogContour::DrawContour(IplImage* iplImg)
{
    cvShowImage("iplImg", iplImg);

    if (outImg) {
        if (outImg->width != iplImg->width || outImg->height != iplImg->height) {
            cvReleaseImage(&outImg);
            outImg = nullptr;
        }
    }

    if (!outImg)
        outImg = cvCreateImage(cvSize(iplImg->width, iplImg->height), IPL_DEPTH_8U, 1);

    CImgProcBase base;
    char text[128];
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

    CvMemStorage* storage = cvCreateMemStorage(0);
    CvSeq* contours = 0;
    cvFindContours(iplImg, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    cvZero(outImg);
    if (!bApproxPoly) {
        if (bContourRect == 0)
            cvDrawContours(outImg, contours, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 2, 1, 8, cvPoint(0, 0));

        while(contours)
        {
            double area = cvContourArea(contours);
            CvRect r = cvBoundingRect(contours);

            if (bContourRect == 1)
                cvDrawRect(outImg, CvPoint(r.x, r.y),CvPoint(r.x+r.width, r.y+r.height), CvScalar(255,255,255), 1, 8);

            if ((r.width+10) >= iplImg->width || (r.height+10) >= iplImg->height) {
                contours = contours->h_next;
                continue;
            }

            if (area > dMinArea)
            {
                cv::Point2f center = base.CenterOfMoment(contours);
                DrawResultCrossMark(outImg, center);

                if (contours->total == 1) {
                    cvRectangle(outImg, CvPoint(0,0), CvPoint(outImg->width/2,20), cvScalar(0, 0, 0), CV_FILLED);
                    sprintf(text, "Area:%.1f", area);
                    cvPutText(outImg, text, cvPoint(10, 16), &font, cvScalar(255, 255, 255));
                }
            }
            contours = contours->h_next;
        }
    }
    else if (bApproxPoly)
    {
        while(contours)
        {
            CvSeq* result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*dEPS, 1);

            cvDrawContours(outImg, result, CVX_WHITE, CVX_WHITE, 1, 1, 8); // 외곽선 근사화가 잘되었는지 테스트

            double area = cvContourArea(result);
            if (area > dMinArea)
            {
                cv::Point2f center = base.CenterOfMoment(result);
                DrawResultCrossMark(outImg, center);

                cvRectangle(outImg, CvPoint(0,0), CvPoint(outImg->width/2,20), cvScalar(0, 0, 0), CV_FILLED);
                sprintf(text, "Area:%.1f", area);
                cvPutText(outImg, text, cvPoint(10, 16), &font, cvScalar(255, 255, 255));
            }

            contours = contours->h_next;
            cvClearSeq(result);
        }
    }

    //cv::imshow("OutImg", cvarrToMat(outImg));


    cvReleaseMemStorage(&storage);

}

void DialogContour::TestMatchShapes(IplImage* iplImg, IplImage* iplImg2)
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

    IplImage* clone1 = cvCreateImage(cvSize(w, h), iplImg->depth, iplImg->nChannels);
    cvZero(clone1);
    IplImage* clone2 = cvCreateImage(cvSize(w, h), iplImg->depth, iplImg->nChannels);
    cvZero(clone2);

    CvRect rect = CvRect(0,0,iplImg->width, iplImg->height);
    cvSetImageROI(clone1, rect);
    cvCopy(iplImg, clone1);
    cvResetImageROI(clone1);

    rect = CvRect(0,0,iplImg2->width, iplImg2->height);
    cvSetImageROI(clone2, rect);
    cvCopy(iplImg2, clone2);
    cvResetImageROI(clone2);


    cvCanny(clone1, clone1, 100, 300, 3);
    cvCanny(clone2, clone2, 100, 300, 3);

    double matching = cvMatchShapes(clone1, clone2, CV_CONTOURS_MATCH_I2);

    char text[128];
    sprintf(text, "Match:%.1f", matching);
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

    cvCopy(clone1, outImg);
    cvOr(clone2, outImg, outImg);

    cvRectangle(outImg, CvPoint(0,0), CvPoint(outImg->width/2,20), cvScalar(255, 255, 255), CV_FILLED);
    cvPutText(outImg, text, cvPoint(10, 16), &font, cvScalar(0, 0, 0));

    //cv::imshow("clone1", cvarrToMat(clone1));
    //cv::imshow("clone2", cvarrToMat(clone2));
    //cv::imshow("tmp", cvarrToMat(outImg));



    cvReleaseImage(&clone1);
    cvReleaseImage(&clone2);
}

void DialogContour::TestShapeContextDistanceExtractor(IplImage* iplImg, IplImage* iplImg2)
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

    IplImage* clone1 = cvCreateImage(cvSize(w, h), iplImg->depth, iplImg->nChannels);
    cvZero(clone1);
    IplImage* clone2 = cvCreateImage(cvSize(w, h), iplImg->depth, iplImg->nChannels);
    cvZero(clone2);

    CvRect rect = CvRect(0,0,iplImg->width, iplImg->height);
    cvSetImageROI(clone1, rect);
    cvCopy(iplImg, clone1);
    cvResetImageROI(clone1);

    rect = CvRect(0,0,iplImg2->width, iplImg2->height);
    cvSetImageROI(clone2, rect);
    cvCopy(iplImg2, clone2);
    cvResetImageROI(clone2);


    double area1=0, area2=0;
    vector<vector<cv::Point> > ca,cb;
    findContours(cv::cvarrToMat(clone1), ca, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_KCOS);
    findContours(cv::cvarrToMat(clone2), cb, cv::RETR_CCOMP, cv::CHAIN_APPROX_TC89_KCOS);

    CvMemStorage *s2 = cvCreateMemStorage(0);
    CvSeq* seq2 = cvCreateSeq((CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_POINT | CV_SEQ_FLAG_CLOSED) | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), s2);

    int first = -1;
    for (size_t i = 0; i <ca.size(); i++) {
        for (size_t j = 0; j <ca[i].size(); j++)
        {
            cv::Point pt = ca[i][j];
            CvPoint *pt2 = new CvPoint(pt.x, pt.y);
            cvSeqPush(seq2, pt2);
        }

        double area = cvContourArea(seq2);
        if (area > dMinArea)
        {
            area1 = area;
            first = i;
            cvDrawContours(outImg, seq2, CVX_WHITE, CVX_WHITE, 1, 1, 8);
        }
        cvClearSeq(seq2);
    }
    int second = -1;
    for (size_t i = 0; i <cb.size(); i++) {
        for (size_t j = 0; j <cb[i].size(); j++)
        {
            cv::Point pt = cb[i][j];
            CvPoint *pt2 = new CvPoint(pt.x, pt.y);
            cvSeqPush(seq2, pt2);
        }

        double area = cvContourArea(seq2);
        if (area > dMinArea)
        {
            area2 = area;
            second = i;
            cvDrawContours(outImg, seq2, CVX_WHITE, CVX_WHITE, 1, 1, 8);
        }
        cvClearSeq(seq2);
    }
    if (s2) cvReleaseMemStorage(&s2);

    if (first >= 0 && second >= 0)
    {
        cv::Ptr<cv::HausdorffDistanceExtractor> hd = cv::createHausdorffDistanceExtractor();
        cv::Ptr<cv::ShapeContextDistanceExtractor> sd = cv::createShapeContextDistanceExtractor();

        double d1 = 0;
        double d2 = 0;
        try {
            d1 = hd->computeDistance(ca[first],cb[second]);
            d2 = sd->computeDistance(ca[first],cb[second]);
        } catch (...) {
            qDebug() << ("Error <unknown> ShapeContextDistanceExtractor");
            return;
        }

        char text[128];
        sprintf(text, "hd:%.1f sd:%.1f %.0f %.0f",  d1, d2, area1, area2);
        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);

        cvRectangle(outImg, CvPoint(0,0), CvPoint(outImg->width/2,20), cvScalar(255, 255, 255), CV_FILLED);
        cvPutText(outImg, text, cvPoint(10, 16), &font, cvScalar(128, 128, 128));
    }

    cvReleaseImage(&clone1);
    cvReleaseImage(&clone2);
}


// https://m.blog.naver.com/PostView.nhn?blogId=cyber3208&logNo=60163282137&proxyReferer=https%3A%2F%2Fwww.google.com%2F
//example_14-04.cpp

