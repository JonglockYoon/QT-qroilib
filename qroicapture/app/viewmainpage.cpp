//
//viewmainpage.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//

// Qt
#include <QDebug>
#include <QMenu>
#include <QStatusBar>

// Local
#include "viewmainpage.h"
#include <qroilib/gvdebug.h>
#include <qroilib/zoomwidget.h>
#include <qroilib/zoommode.h>

#include "mainwindow.h"

using namespace Qroilib;

class DocumentView;

const int ViewMainPage::MaxViewCount = 8;

/*
 * Layout :
 *
 * +-mAdapterContainer-----------------------------------------------+
 * |+-mDocumentViewContainer----------------------------------------+|
 * ||+-DocumentView----------------++-DocumentView-----------------+||
 * |||                             ||                              |||
 * |||                             ||                              |||
 * |||                             ||                              |||
 * |||                             ||                              |||
 * |||                             ||                              |||
 * |||                             ||                              |||
 * ||+-----------------------------++------------------------------+||
 * |+---------------------------------------------------------------+|
 * |+-mStatusBarContainer-------------------------------------------+|
 * ||                                                 [mZoomWidget]|||
 * |+---------------------------------------------------------------+|
 * +-----------------------------------------------------------------+
 */
struct ViewMainPagePrivate
{
    ViewMainPage* q;
    DocumentViewController* mDocumentViewController;
    QList<Qroilib::DocumentView*> mDocumentViews;
    ZoomWidget* mZoomWidget;
    DocumentViewContainer* mDocumentViewContainer;
    QStatusBar* mStatusBarContainer;

    bool mCompareMode;
    ZoomMode::Enum mZoomMode;

    Qroilib::DocumentView* createDocumentView(const QUrl url)
    {
        Qroilib::DocumentView* view = mDocumentViewContainer->createView(url);

        // Connect context menu
        // If you need to connect another view signal, make sure it is disconnected in deleteDocumentView
        QObject::connect(view, &Qroilib::DocumentView::contextMenuRequested, q, &ViewMainPage::showContextMenu);
        QObject::connect(view, &Qroilib::DocumentView::completed, q, &ViewMainPage::completed);
        QObject::connect(view, &Qroilib::DocumentView::focused, q, &ViewMainPage::setCurrentView);

        mDocumentViews << view;

        return view;
    }

    void deleteDocumentView(Qroilib::DocumentView* view)
    {
        if (mDocumentViewController->view() == view) {
            mDocumentViewController->setView(0);
        }

        // Make sure we do not get notified about this view while it is going away.
        // mDocumentViewController->deleteView() animates the view deletion so
        // the view still exists for a short while when we come back to the
        // event loop)
        QObject::disconnect(view, 0, q, 0);

        mDocumentViews.removeOne(view);
        mDocumentViewContainer->deleteView(view);
    }

    void setupStatusBar()
    {
        mStatusBarContainer = new QStatusBar;
        mZoomWidget = new ZoomWidget;

        q->mStatusLabel = new QLabel("Message");
        q->mStatusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        q->mStatusLabel->setText(QString("%1").arg(" ----"));
        mStatusBarContainer->addPermanentWidget(q->mStatusLabel, 1);
        mStatusBarContainer->addPermanentWidget(mZoomWidget);
    }

    Qroilib::DocumentView* currentView() const
    {
        return mDocumentViewController->view();
    }

    void setCurrentView(Qroilib::DocumentView* view)
    {
        Qroilib::DocumentView* oldView = currentView();
        if (view == oldView) {
            return;
        }
        if (oldView) {
            oldView->setCurrent(false);
        }
        view->setCurrent(true);
        mDocumentViewController->setView(view);
    }
};

ViewMainPage::ViewMainPage(QWidget* parent)
: QWidget(parent)
, d(new ViewMainPagePrivate)
{

    myCamCapture = nullptr;

    d->q = this;
    d->mCompareMode = false;

    d->setupStatusBar();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    d->mDocumentViewContainer = new DocumentViewContainer;
    d->mDocumentViewContainer->setAutoFillBackground(true);
    d->mDocumentViewContainer->setBackgroundRole(QPalette::Base);
    layout->addWidget(d->mDocumentViewContainer);
    layout->addWidget(d->mStatusBarContainer);
    objContainer = d->mDocumentViewContainer;

    d->mDocumentViewController = new DocumentViewController(this);
    d->mDocumentViewController->setZoomWidget(d->mZoomWidget);

    const QUrl ch1 = QUrl("file:channel1");
    QList<QUrl> urls;
    urls.append(ch1);
    openUrls(urls, ch1);

    qApp->installEventFilter(this);
}

ViewMainPage::~ViewMainPage()
{
    delete d;
}

void ViewMainPage::loadConfig()
{
    Q_FOREACH(Qroilib::DocumentView * view, d->mDocumentViews) {
        view->loadAdapterConfig();
    }

    d->mZoomMode = ZoomMode::Autofit;
}

int ViewMainPage::statusBarHeight() const
{
    return d->mStatusBarContainer->height();
}

void ViewMainPage::completed(int seq)
{
    qDebug() << "DocumentView complete" << seq;

}

void ViewMainPage::ZoomInU()
{
    Qroilib::DocumentView* v = currentView();
    if (v) {
        qreal z = v->zoom();
        v->setZoom(z+0.1);
    }
}
void ViewMainPage::ZoomOutU()
{
    Qroilib::DocumentView* v = currentView();
    if (v) {
        qreal z = v->zoom();
        if (z > 0.1)
            v->setZoom(z-0.1);
    }
}
void ViewMainPage::FitU()
{
    Qroilib::DocumentView* v = currentView();
    if (v) {
        v->zoomActualSize();
        v->setZoomToFit(true);
    }

}
void ViewMainPage::showContextMenu()
{
    qDebug() << "showContextMenu";

    QMenu contextMenu(tr("Context menu"), this);

    QAction actionZoomIn("Zoom In", this);
    connect(&actionZoomIn, SIGNAL(triggered()), this, SLOT(ZoomInU()));
    contextMenu.addAction(&actionZoomIn);
    QAction actionZoomOut("Zoom Out", this);
    connect(&actionZoomOut, SIGNAL(triggered()), this, SLOT(ZoomOutU()));
    contextMenu.addAction(&actionZoomOut);
    QAction actionFit("Fit", this);
    connect(&actionFit, SIGNAL(triggered()), this, SLOT(FitU()));
    contextMenu.addAction(&actionFit);

   contextMenu.exec(QCursor::pos());
}

//QSize ViewMainPage::sizeHint() const
//{
//    return QSize(MAX_CAMERA_WIDTH, MAX_CAMERA_HEIGHT);
//}

QUrl ViewMainPage::url() const
{
    GV_RETURN_VALUE_IF_FAIL(d->currentView(), QUrl());
    return d->currentView()->url();
}

Qroilib::RasterImageView* ViewMainPage::imageView() const
{
    if (!d->currentView()) {
        return 0;
    }
    return d->currentView()->imageView();
}

//
// 8개까지 multiview를 사용가능..
//
void ViewMainPage::openUrls(const QList<QUrl>& allUrls, const QUrl &currentUrl)
{
    Qroilib::DocumentView::Setup setup;

    //QSet<QUrl> urls = allUrls.toSet();

    d->mCompareMode = allUrls.count() > 1;

    setup.valid = true;
    setup.zoomToFit = false;//true;
    setup.zoom = 1.0;

    int seq = 0;
    // Create view for remaining urls
    Q_FOREACH(const QUrl &url, allUrls) {
        if (d->mDocumentViews.count() >= MaxViewCount) {
            qWarning() << "Too many documents to show";
            break;
        }
        Qroilib::DocumentView* view = d->createDocumentView(url);
        view->bMultiView = false;
        view->seq = seq;
        seq++;
    }
    d->mDocumentViewContainer->updateLayout();

    Q_FOREACH(Qroilib::DocumentView * view, d->mDocumentViews) {
        view->openUrl(allUrls[view->seq], setup, view->seq);
    }

    // Init views
    Q_FOREACH(Qroilib::DocumentView * view, d->mDocumentViews) {
        view->setCompareMode(d->mCompareMode);
        QUrl url = view->url();
        if (url == currentUrl) {
            d->setCurrentView(view);
        } else {
            view->setCurrent(false);
        }
    }
}

void ViewMainPage::updateLayout()
{
    d->mDocumentViewContainer->updateLayout();
}

void ViewMainPage::reset()
{
    d->mDocumentViewController->setView(0);
    d->mDocumentViewContainer->reset();
    d->mDocumentViews.clear();

    Q_FOREACH(Qroilib::DocumentView * view, d->mDocumentViews) {
        delete view;
    }
}

void ViewMainPage::setCurrentView(Qroilib::DocumentView* view)
{
    d->setCurrentView(view);
}

Qroilib::DocumentView* ViewMainPage::currentView() const
{
    return d->currentView();
}

Qroilib::DocumentView* ViewMainPage::view(int n) const
{
    if (d->mDocumentViews.size() <= n)
        return nullptr;
    return d->mDocumentViews[n];
}


bool ViewMainPage::eventFilter(QObject *obj, QEvent *event)
{
    //Q_UNUSED(obj);
    //Q_UNUSED(event);

    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        QObject* tmp = objContainer;
        if (obj->parent() != tmp) {
            mStatusLabel->setText("");
            return false;
        }

        Qroilib::DocumentView *v = (Qroilib::DocumentView *)currentView();
        if (!v)
            return false;
        const RasterImageView* imgView = imageView();
        if (imgView) {
            //qDebug() << obj;
            const QImage *pimg = v->image();
            //const QPointF pt = imgView->getLastMousePos();
            int x = mouseEvent->pos().x();
            int y = mouseEvent->pos().y();
            const QPointF pt1 = imgView->mImageOffset;
            x = x - pt1.x();
            y = y - pt1.y();
            const QPointF pt2 = imgView->mScrollPos;
            x = x + pt2.x();
            y = y + pt2.y();

            const qreal z = v->zoom();
            x = x / z;
            y = y / z;

            if (x >= 0 && y >= 0 && x < pimg->width() && y < pimg->height())
            {
                QRgb c = pimg->pixel(x, y);
                const QColor src(c);

                int r = src.red();
                int g = src.green();
                int b = src.blue();

                cv::Mat HSV;
                cv::Mat RGB(1,1,CV_8UC3,cv::Scalar(b,g,r));
                cv::cvtColor(RGB, HSV,CV_BGR2HSV);
                cv::Vec3b hsv=HSV.at<cv::Vec3b>(0,0);
                int H=(int)hsv.val[0]; // 0 ~ 179
                cv::Mat GRAY;
                cv::cvtColor(RGB, GRAY,CV_BGR2GRAY);
                cv::Vec3b gy=GRAY.at<cv::Vec3b>(0,0);

                QString str, str1;
                str.sprintf("x:%d y:%d  r:%d g:%d b:%d", x,y ,r,g,b);
                //double gray = r * 0.299f + g * 0.587f + b * 0.114f; // https://docs.opencv.org/3.3.0/de/d25/imgproc_color_conversions.html
                //if (gray < 0.0) gray = 0;
                //if (gray > 255.0) gray = 255;
                str1.sprintf(" Hue:%d Gray: %d", H, gy.val[0]);
                str += str1;

                mStatusLabel->setText(str);
            }
            else mStatusLabel->setText("");
        }
        else
            mStatusLabel->setText(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
    }

    return false;
}

void ViewMainPage::OpenCam(int seq, int nCam)
{
    myCamCapture = new CamCapture(d->mDocumentViews[seq], seq, nCam);
    //qDebug() << "myCamCapture seq: " << seq;
    QObject::connect(myCamCapture, SIGNAL(processedImage(const QImage&, int)), this, SLOT(updatePlayerUI(const QImage&, int)));

    if (myCamCapture->OpenCam() == true)
        myCamCapture->Play();
}
void ViewMainPage::CloseCam(int seq)
{
    if (myCamCapture)
    {
        QObject::disconnect(myCamCapture, SIGNAL(processedImage(const QImage&, int)), this, SLOT(updatePlayerUI(const QImage&, int)));
        myCamCapture->Stop();
        QThread::msleep(300);
        myCamCapture->exit();
        myCamCapture->CloseCam();
        delete myCamCapture;
        myCamCapture = nullptr;
    }
}

void ViewMainPage::updatePlayerUI(const QImage& img, int seq)
{
    QMutexLocker lock(&m_sync);

    Qroilib::DocumentView* v = view(seq);
    if (v == nullptr)
        return;

    if (!v->isVisible())
        return;

    if (!v->document())
        return;

    v->document()->setImageInternal(img);
    v->imageView()->updateBuffer();
}

