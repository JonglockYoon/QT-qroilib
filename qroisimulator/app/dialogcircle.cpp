#include "dialogcircle.h"
#include <QBoxLayout>
#include <QDebug>
#include "mainwindow.h"
#include "ui_dialogcircle.h"

DialogCircle::DialogCircle(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCircle)
{
    QString str;
    QString date = QDateTime::currentDateTime().toString("yyyyMMdd");
    QString time = QDateTime::currentDateTime().toString("hhmmsszzz");
    mName = QString("RansacCircle%1%2").arg(date).arg(time);
    setWindowTitle(mName);

    ui->setupUi(this);

    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(0);


    connect(ui->comboBoxSource, SIGNAL(activated(int)), this, SLOT(activatedComboBoxSource(int)));
    QObject::connect(ui->chkBoxRealtime, SIGNAL(clicked(bool)), this, SLOT(changeRealtime(bool)));

    ui->sliderp1->setMinimum(1);
    ui->sliderp1->setMaximum(100);
    ui->sliderp1->setSingleStep(1);
    ui->sliderp1->setPageStep(10);

    bestCirclePercentage = 0.5;
    QObject::connect(ui->editp1, SIGNAL(textChanged(const QString &)), this, SLOT(setEditValuep1(const QString &)));
    QObject::connect(ui->sliderp1, SIGNAL(valueChanged(int)), this, SLOT(setValuep1(int)));

    str = QString("%1").arg(bestCirclePercentage * 100.0);
    ui->editp1->setText(str);
}

DialogCircle::~DialogCircle()
{
}

void DialogCircle::closeEvent(QCloseEvent *event)
{
    if (source == 0)
    {
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        if (pView) {
            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    } else {
//        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
//        ViewOutPage* pView = pWidget->mViewOutPage;
//        if (pView) {
//            QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
//        }
    }

    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
}

void DialogCircle::changeRealtime(bool checked)
{
    bRealtime = checked;

    if (source == 0)
    {
        ViewMainPage *pView = (ViewMainPage *)theMainWindow->viewMainPage();
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            if (bRealtime)
                QObject::connect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
            else
                QObject::disconnect(pView, SIGNAL(processedImage(const QImage*)), this, SLOT(updatePlayerUI(const QImage*)));
        }
    }
}

void DialogCircle::activatedComboBoxSource(int act)
{
    source = act;
}
void DialogCircle::setValuep1(int val)
{
    bestCirclePercentage = (float)val / 100.0;
    QString str = QString("%1").arg(val);
    ui->editp1->setText(str);
}
void DialogCircle::setEditValuep1(const QString &val)
{
    bestCirclePercentage = (float)val.toInt() / 100.0;
    ui->sliderp1->setValue(val.toInt());
}

void DialogCircle::on_pushButtonClose_clicked()
{
    if (tmp)
        cvReleaseImage(&tmp);
    tmp = nullptr;
    hide();
}


float DialogCircle::verifyCircle(cv::Mat dt, cv::Point2f center, float radius, std::vector<cv::Point2f> & inlierSet)
{
    unsigned int counter = 0;
    unsigned int inlier = 0;
    float minInlierDist = 2.0f;
    float maxInlierDistMax = 100.0f;
    float maxInlierDist = radius / 25.0f;
    if (maxInlierDist<minInlierDist) maxInlierDist = minInlierDist;
    if (maxInlierDist>maxInlierDistMax) maxInlierDist = maxInlierDistMax;

    // choose samples along the circle and count inlier percentage
    for (float t = 0; t<2 * 3.14159265359f; t += 0.05f)
    {
        counter++;
        float cX = radius*cos(t) + center.x;
        float cY = radius*sin(t) + center.y;

        if (cX < dt.cols)
            if (cX >= 0)
                if (cY < dt.rows)
                    if (cY >= 0)
                        if (dt.at<float>(cY, cX) < maxInlierDist)
                        {
                            inlier++;
                            inlierSet.push_back(cv::Point2f(cX, cY));
                        }
    }

    return (float)inlier / float(counter);
}


inline void DialogCircle::getCircle(cv::Point2f& p1, cv::Point2f& p2, cv::Point2f& p3, cv::Point2f& center, float& radius)
{
    float x1 = p1.x;
    float x2 = p2.x;
    float x3 = p3.x;

    float y1 = p1.y;
    float y2 = p2.y;
    float y3 = p3.y;

    // PLEASE CHECK FOR TYPOS IN THE FORMULA :)
    center.x = (x1*x1 + y1*y1)*(y2 - y3) + (x2*x2 + y2*y2)*(y3 - y1) + (x3*x3 + y3*y3)*(y1 - y2);
    center.x /= (2 * (x1*(y2 - y3) - y1*(x2 - x3) + x2*y3 - x3*y2));

    center.y = (x1*x1 + y1*y1)*(x3 - x2) + (x2*x2 + y2*y2)*(x1 - x3) + (x3*x3 + y3*y3)*(x2 - x1);
    center.y /= (2 * (x1*(y2 - y3) - y1*(x2 - x3) + x2*y3 - x3*y2));

    radius = sqrt((center.x - x1)*(center.x - x1) + (center.y - y1)*(center.y - y1));
}

std::vector<cv::Point2f> DialogCircle::getPointPositions(cv::Mat binaryImage)
{
    std::vector<cv::Point2f> pointPositions;

    for (int y = 0; y<binaryImage.rows; ++y)
    {
        for (int x = 0; x<binaryImage.cols; ++x)
        {
            if (binaryImage.at<unsigned char>(y, x) > 0) pointPositions.push_back(cv::Point2f(x, y));
        }
    }

    return pointPositions;
}

// bestCirclePercentage : 0 ~ 1.0
int DialogCircle::Find_RANSAC_Circle(IplImage* grayImgIn)
{
    //QString str;
    IplImage* grayImg = cvCreateImage(cvGetSize(grayImgIn), IPL_DEPTH_8U, 1);
    CBlobResult blobs = CBlobResult(grayImgIn, nullptr, 0);
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++)	// 여러개의 Blob이 있을때 한개씩 뽑아서 RANSAC을 돌린다.
    {
        CBlob *p = blobs.GetBlob(i);
        CvBox2D d = p->GetEllipse();
        cvZero(grayImg);
        p->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.

        //RANSAC
        cv::Mat canny;
        cv::Mat gray = cv::cvarrToMat(grayImg);

        cv::Mat mask;

        float canny1 = 100;
        float canny2 = 20;

        cv::Canny(gray, canny, canny1, canny2);

        mask = canny;

        std::vector<cv::Point2f> edgePositions;
        edgePositions = getPointPositions(mask);

        // create distance transform to efficiently evaluate distance to nearest edge
        cv::Mat dt;
        cv::distanceTransform(255 - mask, dt, CV_DIST_L1, 3);

        int maxNrOfIterations = edgePositions.size();

        RANSACIRCLE rbest;
        rbest.cPerc = 0.0;
        rbest.center.x = 0;
        for (int its = 0; its < maxNrOfIterations; ++its)
        {
            unsigned int idx1 = rand() % edgePositions.size();
            unsigned int idx2 = rand() % edgePositions.size();
            unsigned int idx3 = rand() % edgePositions.size();

            // we need 3 different samples:
            if (idx1 == idx2) continue;
            if (idx1 == idx3) continue;
            if (idx3 == idx2) continue;

            // create circle from 3 points:
            cv::Point2f center; float radius;
            getCircle(edgePositions[idx1], edgePositions[idx2], edgePositions[idx3], center, radius);

            // inlier set unused at the moment but could be used to approximate a (more robust) circle from alle inlier
            std::vector<cv::Point2f> inlierSet;

            //verify or falsify the circle by inlier counting:
            float cPerc = verifyCircle(dt, center, radius, inlierSet);

            // update best circle information if necessary
            QRect rect = QRect(QPoint(center.x-radius, center.y-radius), QPoint(center.x+radius, center.y+radius));
            // circle 영역안에 blob center가 포함되어야한다.
            if (rect.contains(d.center.x, d.center.y))
            {
                RANSACIRCLE rclc;
                rclc.cPerc = cPerc;
                if (rbest.cPerc < rclc.cPerc) {
                    rbest.cPerc = rclc.cPerc;
                    rbest.center = center;
                    rbest.radius = radius;
                }
            }
        }
        if (rbest.center.x > 0) {
            if (rbest.cPerc > bestCirclePercentage) {
                vecRansicCircle.push_back(rbest);
            }
        }
    }
    cvReleaseImage(&grayImg);

    return 0;
}

//
// bRealtime 처리..
//
void DialogCircle::updatePlayerUI(const QImage* img)
{
    on_pushButtonExec_clicked();
}
void DialogCircle::on_pushButtonExec_clicked()
{
    IplImage* iplImg = nullptr;
    if (source == 0)
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
                    ExecRansacCircle(iplImg);
                    return;
                }

                IplImage* croppedImage;
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

                ExecRansacCircle(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecRansacCircle(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecRansacCircle(iplImg);
        }
    }
}

void DialogCircle::ExecRansacCircle(IplImage* iplImg)
{
    if (tmp) {
        if (tmp->width != iplImg->width || tmp->height != iplImg->height) {
            cvReleaseImage(&tmp);
            tmp = nullptr;
        }
    }

    if (!tmp)
        tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), iplImg->depth, iplImg->nChannels);
    cvCopy(iplImg, tmp);
    //cvZero(tmp);

    vecRansicCircle.clear();

    if (Find_RANSAC_Circle(iplImg) == 0)
    {
        int size = vecRansicCircle.size();
        for (int i = size - 1; i >= 0; i--)
        {
            RANSACIRCLE *p = &vecRansicCircle[i];

            cvCircle(tmp, cvPoint(p->center.x, p->center.y), p->radius, CV_RGB(126, 126, 126), 2);
        }
    }

    theMainWindow->outWidget(mName, tmp);
}
