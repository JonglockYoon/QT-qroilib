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

    method = 0;
    ui->radioButton1->setChecked(true);
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


    ui->comboBoxSource->clear();
    ui->comboBoxSource->insertItem(0, QIcon(), QString::fromLocal8Bit("qroisimulator"));
    int size = theMainWindow->vecOutWidget.size();
    for (int i=0; i<size; i++) {
        OutWidget* pWidget = theMainWindow->vecOutWidget[i];
        ui->comboBoxSource->insertItem(i+1, QIcon(), pWidget->windowTitle());
    }
    ui->comboBoxSource->setCurrentIndex(act);
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
    float iBestCP = bestCirclePercentage;
    //QString str;
    IplImage* grayImg = cvCreateImage(cvGetSize(grayImgIn), IPL_DEPTH_8U, 1);
    CBlobResult blobs = CBlobResult(grayImgIn, nullptr);
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++)	// 여러개의 Blob이 있을때 한개씩 뽑아서 RANSAC을 돌린다.
    {
        CBlob *p = blobs.GetBlob(i);
        CvBox2D d = p->GetEllipse();
        cvZero(grayImg);
        p->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.

        int nGaussian = 7;
        try {
            cvSmooth(grayImg, grayImg, CV_GAUSSIAN,nGaussian,nGaussian);
        } catch (...) {
            qDebug() << "Error cvSmooth()";
            continue;
        }

        //RANSAC
        cv::Mat canny;
        cv::Mat gray = cv::cvarrToMat(grayImg);

        cv::Mat mask;

        float canny1 = 100;
        float canny2 = 200;

        cv::Canny(gray, canny, canny1, canny2);

        mask = canny;

        std::vector<cv::Point2f> edgePositions;
        edgePositions = getPointPositions(mask);

        // create distance transform to efficiently evaluate distance to nearest edge
        cv::Mat dt;
        cv::distanceTransform(255 - mask, dt, CV_DIST_L1, 3);
//cv::imshow("mask",mask);
//cv::imshow("255-mask",255-mask);
//cv::Mat dtc = dt.clone();
//cv::normalize(dtc, dtc, 0, 1., cv::NORM_MINMAX);
//cv::imshow("dt",dtc);
        int maxNrOfIterations = edgePositions.size();
again:
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
                rclc.center = center;
                rclc.radius = radius;
                if (rbest.cPerc < rclc.cPerc) {
                    rbest = rclc;
                }
                if (rclc.cPerc > iBestCP) {
                    vecRansicCircle.push_back(rclc);
                }
            }
        }
        if (vecRansicCircle.size() == 0) {
            iBestCP = iBestCP - 0.05;
            if (iBestCP > 0.1)
                goto again;
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
                    ExecCircleFit(iplImg);
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

                ExecCircleFit(croppedImage);

                cvReleaseImage(&croppedImage);
            }
            else
                ExecCircleFit(iplImg);
        }
    } else {
        OutWidget* pWidget = theMainWindow->vecOutWidget[source-1];
        ViewOutPage* pView = pWidget->mViewOutPage;
        if (pView) {
            iplImg = pView->getIplgray();
            if (!iplImg)
                return;
            ExecCircleFit(iplImg);
        }
    }
}

void DialogCircle::on_radioButton1_clicked()
{
    method = 0;
}
void DialogCircle::on_radioButton2_clicked()
{
    method = 1;
}
void DialogCircle::on_radioButton3_clicked()
{
    method = 2;
}

void DialogCircle::ExecRansacCircle(IplImage* iplImg)
{
again:
    vecRansicCircle.clear();

    if (Find_RANSAC_Circle(iplImg) == 0)
    {
        int size = vecRansicCircle.size();

        std::stable_sort(vecRansicCircle.begin(), vecRansicCircle.end(), [](const RANSACIRCLE lhs, const RANSACIRCLE rhs)->bool {
            if (lhs.cPerc > rhs.cPerc) // descending
                return true;
            return false;
        });


        if (size <= 1)
            goto again;

        if (size > 2) {
            float r1 = vecRansicCircle[0].radius;
            float r2 = vecRansicCircle[1].radius;
            if (fabs(r1-r2) < 3.0) // First, Second Radius 차가 큰것은 재 시도.
                goto again;
        }

        char text[128] = { 0 };
        sprintf(text, "%.2f, %.2f / %d",  vecRansicCircle[0].cPerc, vecRansicCircle[0].radius, size);
        CvFont font;
        cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, CV_AA);
        cvPutText(tmp, text, cvPoint(10, 10), &font, cvScalar(128, 128, 128));

        RANSACIRCLE *p = &vecRansicCircle[0];
        cvCircle(tmp, cvPoint(p->center.x, p->center.y), p->radius, CV_RGB(126, 126, 126), 2);
    }

}

void DialogCircle::ExecFitEllipse2(IplImage* iplImg)
{
    int dilation_type = 0;
    int dilation_elem = 0;

    if (dilation_elem == 0) { dilation_type = cv::MORPH_RECT; }
    else if (dilation_elem == 1) { dilation_type = cv::MORPH_CROSS; }
    else if (dilation_elem == 2) { dilation_type = cv::MORPH_ELLIPSE; }

    int size = 1;

    cv::Mat img = cv::cvarrToMat(iplImg);
    cv::Mat element = cv::getStructuringElement(dilation_type, cv::Size(2 * size + 1, 2 * size + 1), cv::Point(size, size));
    cv::morphologyEx(img, img, cv::MORPH_OPEN, element);

    IplImage* grayImg = cvCreateImage(cvGetSize(iplImg), IPL_DEPTH_8U, 1);
    CBlobResult blobs = CBlobResult(iplImg, nullptr);
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++)	// 여러개의 Blob이 있을때 한개씩 뽑아서 RANSAC을 돌린다.
    {
        CBlob *p = blobs.GetBlob(i);
        CvBox2D d = p->GetEllipse();
        cvZero(grayImg);
        p->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.

        cv::Mat img1 = cv::cvarrToMat(grayImg);
        cv::Canny(img1, img1, 100, 300);

        CvMemStorage* m_storage = cvCreateMemStorage(0); // 배열 자료(점의 좌표가 들어간다)
        CvSeq* ptseq;
        ptseq = cvCreateSeq((CV_SEQ_KIND_CURVE | CV_SEQ_ELTYPE_POINT | CV_SEQ_FLAG_CLOSED) | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);
        for (int x = 0; x < img1.cols; x++)
        {
            for (int y = 0; y < img1.rows; y++)
            {
                if (img1.at<uchar>(y, x) > 0)
                {
                    CvPoint temp;
                    temp.x = x;
                    temp.y = y;
                    cvSeqPush(ptseq, &temp);
                }
            }
        }

        CvBox2D box2d = cvFitEllipse2(ptseq); // : 2D의 점들에서 가장 잘 맞는 타원을 찾는다.
        cvEllipseBox(tmp,box2d,cv::Scalar(128,128,128),2,8,0);
        cvClearSeq(ptseq);
        if (m_storage) cvReleaseMemStorage(&m_storage);
    }
    cvReleaseImage(&grayImg);
}

void DialogCircle::ExecLeastSquare(IplImage* iplImg)
{
    int dilation_type = 0;
    int dilation_elem = 0;

    if (dilation_elem == 0) { dilation_type = cv::MORPH_RECT; }
    else if (dilation_elem == 1) { dilation_type = cv::MORPH_CROSS; }
    else if (dilation_elem == 2) { dilation_type = cv::MORPH_ELLIPSE; }

    int size = 1;

    cv::Mat img = cv::cvarrToMat(iplImg);
    cv::Mat element = cv::getStructuringElement(dilation_type, cv::Size(2 * size + 1, 2 * size + 1), cv::Point(size, size));
    cv::morphologyEx(img, img, cv::MORPH_OPEN, element);

    IplImage* grayImg = cvCreateImage(cvGetSize(iplImg), IPL_DEPTH_8U, 1);
    CBlobResult blobs = CBlobResult(iplImg, nullptr);
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++)	// 여러개의 Blob이 있을때 한개씩 뽑아서 RANSAC을 돌린다.
    {
        CBlob *p = blobs.GetBlob(i);
        CvBox2D d = p->GetEllipse();
        cvZero(grayImg);
        p->FillBlob(grayImg, CVX_WHITE);	// Draw the filtered blobs as white.

        cv::Mat img1 = cv::cvarrToMat(grayImg);
        cv::Canny(img1, img1, 100, 300);

        vector<cv::Point2f> points;
        for (int x = 0; x < img1.cols; x++)
        {
            for (int y = 0; y < img1.rows; y++)
            {
                if (img1.at<uchar>(y, x) > 0)
                {
                    points.push_back(cv::Point2f(x, y));
                }
            }
        }

        // Least Square Algorithm

        float xn = 0, xsum = 0;
        float yn = 0, ysum = 0;
        float n = points.size();

        for (int i = 0; i < n; i++)
        {
            xsum = xsum + points[i].x;
            ysum = ysum + points[i].y;
        }

        xn = xsum / n;
        yn = ysum / n;

        float ui = 0;
        float vi = 0;
        float suu = 0, suuu = 0;
        float svv = 0, svvv = 0;
        float suv = 0;
        float suvv = 0, svuu = 0;

        for (int i = 0; i < n; i++)
        {
            ui = points[i].x - xn;
            vi = points[i].y - yn;

            suu = suu + (ui * ui);
            suuu = suuu + (ui * ui * ui);

            svv = svv + (vi * vi);
            svvv = svvv + (vi * vi * vi);

            suv = suv + (ui * vi);

            suvv = suvv + (ui * vi * vi);
            svuu = svuu + (vi * ui * ui);
        }

        cv::Mat A = (cv::Mat_<float>(2, 2) << suu, suv, suv, svv);

        cv::Mat B = (cv::Mat_<float>(2, 1) << 0.5*(suuu + suvv), 0.5*(svvv + svuu));

        cv::Mat abc;
        cv::solve(A, B, abc);

        float u = abc.at<float>(0);
        float v = abc.at<float>(1);

        float x = u + xn;
        float y = v + yn;

        float alpha = u * u + v * v + ((suu + svv) / n);
        float r = sqrt(alpha);

        // Draw circle
        cv::circle(img, cv::Point(x, y), r, cv::Scalar(128, 128, 128), 2, 8, 0);

    }
    cvCopy(iplImg, tmp);
    cvReleaseImage(&grayImg);
}

void DialogCircle::ExecCircleFit(IplImage* iplImg)
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

    switch (method)
    {
    case 0:
        ExecRansacCircle(iplImg);
        break;
    case 1:
        ExecFitEllipse2(iplImg);
        break;
    case 2:
        ExecLeastSquare(iplImg);
        break;
    }

    theMainWindow->outWidget(mName, tmp);
}
