
// Qt
#include <QCheckBox>
#include <QItemSelectionModel>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QMenu>
#include <QStatusBar>

#include <QMediaMetaData>
#include <QCamera>
#include <QCameraInfo>

// Local
#include "viewmainpage.h"
#include <qroilib/gvdebug.h>
#include <qroilib/paintutils.h>
#include <qroilib/statusbartoolbutton.h>
#include <qroilib/zoomwidget.h>
#include <qroilib/zoommode.h>

#include <fcntl.h>
#ifndef Q_OS_WIN
#include <libv4l1.h>
#include <libv4l2.h>
#endif
#include "hexagonalrenderer.h"
#include "isometricrenderer.h"
#include "orthogonalrenderer.h"
#include "roiobjectitem.h"
#include "addremoveroiobject.h"
#include "roipropertyeditor.h"
#include "mainwindow.h"
#include "config.h"

#undef ENABLE_LOG
#undef LOG
//#define ENABLE_LOG
#ifdef ENABLE_LOG
#define LOG(x) qDebug() << x
#else
#define LOG(x) ;
#endif

//extern MainWindow* theMainWindow;

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
    myCamCapture[0] = nullptr;
    myCamCapture[1] = nullptr;
    roipropertyeditor = nullptr;

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
    const QUrl ch2 = QUrl("file:channel2");
    QList<QUrl> urls;
    urls.append(ch1);
    if (gCfg.m_nCamNumber > 1)
        urls.append(ch2);
    openUrls(urls, ch1);
    qApp->installEventFilter(this);
}

ViewMainPage::~ViewMainPage()
{
    if (roipropertyeditor)
        delete roipropertyeditor;

    for (int i=0; i<2; i++) {
        if ( myCamCapture[i]) {
            myCamCapture[i]->Stop();
            QThread::msleep(200);
            delete myCamCapture[i];
            myCamCapture[i] = nullptr;
        }
    }

    delete d;
}

void ViewMainPage::updatePlayerUI(const QImage& img, int seq)
{
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

//inline void addActionToMenu(QMenu* menu, const char* name)
//{
//    QAction* action = actionCollection->action(name);
//    if (action) {
//        menu->addAction(action);
//    }
//}

void ViewMainPage::completed(int seq)
{
    qDebug() << "DocumentView complete" << seq;
    //OpenCam(seq);
#if 1
    QStringList List;
    List << "None";

    int i = 0;
    int count = List.count();
    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        i++;
        QString str;
        str.sprintf("%02d:%s", i, cameraInfo.description().toLatin1().data());
        List << str;
    }

#ifndef Q_OS_WIN
    {
        char video[64];
        for (int i=0; i<12; i++) {
            sprintf(video, "/dev/video%d", i);
            int fd = ::v4l2_open(video,O_RDWR);
            if (fd != -1) {
                sprintf(video, "%02d:/dev/video%d", i+1, i);
                List << video;
                ::v4l2_close(fd);
            }
        }
    }
#endif
    count = List.count();
    qDebug() << "ViewMainPage::completed" << seq << gCfg.m_sCamera1 << gCfg.m_sCamera2;
    for (int i=1; i<count; i++) {
        if (seq == 0 && List[i] == gCfg.m_sCamera1) {
            QStringList l = gCfg.m_sCamera1.split(":");
            if (l.count() > 0) {
                int nCam = l[0].toInt() - 1;
                OpenCam(seq, nCam);
            }
        }
        else if (seq == 1 && List[i] == gCfg.m_sCamera2) {
            QStringList l = gCfg.m_sCamera2.split(":");
            if (l.count() > 0) {
                int nCam = l[0].toInt() - 1;
                OpenCam(seq, nCam);
            }
        }
    }
#endif
}

void ViewMainPage::OpenCam(int seq, int nCam)
{
    myCamCapture[seq] = new CamCapture(d->mDocumentViews[seq], seq, nCam);
    //qDebug() << "myCamCapture seq: " << seq;
    QObject::connect(myCamCapture[seq], SIGNAL(processedImage(const QImage&, int)), this, SLOT(updatePlayerUI(const QImage&, int)));

    if (myCamCapture[seq]->OpenCam() == true)
        myCamCapture[seq]->Play();
}
void ViewMainPage::CloseCam(int seq)
{
    if (myCamCapture[seq])
    {
        QObject::disconnect(myCamCapture[seq], SIGNAL(processedImage(const QImage&, int)), this, SLOT(updatePlayerUI(const QImage&, int)));
        myCamCapture[seq]->Stop();
        myCamCapture[seq]->exit();
        myCamCapture[seq]->CloseCam();
        delete myCamCapture[seq];
        myCamCapture[seq] = nullptr;
    }
}

void ViewMainPage::showContextMenu()
{
//    QMenu menu(this);
//    if (d->currentView()->canZoom()) {
//        addActionToMenu(&menu, d->mActionCollection, "view_actual_size");
//        addActionToMenu(&menu, d->mActionCollection, "view_zoom_to_fit");
//        addActionToMenu(&menu, d->mActionCollection, "view_zoom_in");
//        addActionToMenu(&menu, d->mActionCollection, "view_zoom_out");
//    }
//    menu.exec(QCursor::pos());
}

QSize ViewMainPage::sizeHint() const
{
    return QSize(640, 480);
}

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
        view->bMultiView = true;
        view->seq = seq;
        seq++;
    }
    d->mDocumentViewContainer->updateLayout();

    Q_FOREACH(Qroilib::DocumentView * view, d->mDocumentViews) {
        view->openUrl(allUrls[view->seq], setup, view->seq);

        connect(view, &Qroilib::DocumentView::editCurrentObject, this, &ViewMainPage::editRoiProperty);
    }

    // Init views
    Q_FOREACH(Qroilib::DocumentView * view, d->mDocumentViews) {
        view->setCompareMode(d->mCompareMode);
        if (view->url() == currentUrl) {
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
void ViewMainPage::editRoiProperty()
{
    Qroilib::DocumentView *v = currentView();
    Qroilib::RoiObject *mObject;
    if (v->mSelectedObjects.size() == 1) {
          mObject = v->mSelectedObjects.first();
    } else
        return;

    if (roipropertyeditor != nullptr) {
        delete roipropertyeditor;
    }
    roipropertyeditor = new RoiPropertyEditor(this, mObject);
    roipropertyeditor->show();
    roipropertyeditor->raise();

}

Qroilib::DocumentView* ViewMainPage::view(int n) const
{
    if (d->mDocumentViews.size() <= n)
        return nullptr;
    return d->mDocumentViews[n];
}
