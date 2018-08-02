//
//ViewOutPage.cpp
//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//

// Qt
#include <QApplication>
#include <QFileDialog>
#include <QDebug>
#include <QMenu>
#include <QStatusBar>

// Local
#include "viewoutpage.h"
#include <qroilib/gvdebug.h>
#include <qroilib/zoomwidget.h>
#include <qroilib/zoommode.h>
#include <qroilib/documentview/documentview.h>

#include "mainwindow.h"
//#include "roipropertyeditor.h"
#include "common.h"

using namespace Qroilib;

class DocumentView;

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
struct ViewOutPagePrivate
{
    ViewOutPage* q;
    DocumentViewController* mDocumentViewController;
    QList<Qroilib::DocumentView*> mDocumentViews;
    ZoomWidget* mZoomWidget;
    DocumentViewContainer* mDocumentViewContainer;
    QStatusBar* mStatusBarContainer;

    ZoomMode::Enum mZoomMode;

    Qroilib::DocumentView* createDocumentView(const QUrl url)
    {
        Qroilib::DocumentView* view = mDocumentViewContainer->createView(url);

        // Connect context menu
        // If you need to connect another view signal, make sure it is disconnected in deleteDocumentView
        QObject::connect(view, &Qroilib::DocumentView::contextMenuRequested, q, &ViewOutPage::showContextMenu);
        QObject::connect(view, &Qroilib::DocumentView::completed, q, &ViewOutPage::completed);
        QObject::connect(view, &Qroilib::DocumentView::focused, q, &ViewOutPage::setCurrentView);

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

ViewOutPage::ViewOutPage(QWidget* parent)
: QWidget(parent)
, d(new ViewOutPagePrivate)
{

    d->q = this;

    d->setupStatusBar();
    d->mZoomMode = ZoomMode::Autofit;

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

    Qroilib::DocumentView::Setup setup;

    setup.valid = true;
    setup.zoomToFit = false;//true;
    setup.zoom = 1.0;

    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString name = "file:" + date + time; // create unique name.
    const QUrl ch1 = QUrl(name);

    Qroilib::DocumentView* view = d->createDocumentView(ch1);
    view->bMultiView = false;
    view->seq = 0;

    d->mDocumentViewContainer->updateLayout();
    view->openUrl(ch1, setup, 0);
    d->setCurrentView(view);

    qApp->installEventFilter(this);
}

ViewOutPage::~ViewOutPage()
{
    delete d;
}

int ViewOutPage::statusBarHeight() const
{
    return d->mStatusBarContainer->height();
}

void ViewOutPage::completed(int seq)
{
    qDebug() << "DocumentView complete" << seq;

}

void ViewOutPage::ZoomInU()
{
    Qroilib::DocumentView* v = currentView();
    if (v) {
        qreal z = v->zoom();
        v->setZoom(z+0.1);
    }
}
void ViewOutPage::ZoomOutU()
{
    Qroilib::DocumentView* v = currentView();
    if (v) {
        qreal z = v->zoom();
        if (z > 0.1)
            v->setZoom(z-0.1);
    }
}
void ViewOutPage::FitU()
{
    Qroilib::DocumentView* v = currentView();
    if (v) {
        v->zoomActualSize();
        v->setZoomToFit(true);
    }

}
void ViewOutPage::SaveU()
{
    Qroilib::DocumentView *v = (Qroilib::DocumentView *)currentView();
    if (v) {
        const QImage *pimg = v->image();
        if (!pimg)
            return;

        QApplication::setOverrideCursor(Qt::WaitCursor);
        QString fileName = QFileDialog::getSaveFileName(this,
                tr("Save Image"), "",
                tr("Images (*.png *.bmp *.jpg *.tif *.ppm *.GIF);;All Files (*)"));
        if (fileName.isEmpty()) {
            return;
        }
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QTimer::singleShot(1, [=] {
            pimg->save(fileName);
            QApplication::restoreOverrideCursor();
        });
    }
}
void ViewOutPage::SetROIU()
{
    Qroilib::DocumentView* v = currentView();
    if (!v)
        return;

    v->selectTool(v->actCreateRectangle);
}
void ViewOutPage::showContextMenu()
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
    QAction actionSaveU("Save", this);
    connect(&actionSaveU, SIGNAL(triggered()), this, SLOT(SaveU()));
    contextMenu.addAction(&actionSaveU);
    //QAction actionSetROIU("SetROI", this); // not yes implemented.
    //connect(&actionSetROIU, SIGNAL(triggered()), this, SLOT(SetROIU()));
    //contextMenu.addAction(&actionSetROIU);
    contextMenu.exec(QCursor::pos());
}

QSize ViewOutPage::sizeHint() const
{
    return QSize(MAX_CAMERA_WIDTH, MAX_CAMERA_HEIGHT);
}

QUrl ViewOutPage::url() const
{
    GV_RETURN_VALUE_IF_FAIL(d->currentView(), QUrl());
    return d->currentView()->url();
}

Qroilib::RasterImageView* ViewOutPage::imageView() const
{
    if (!d->currentView()) {
        return 0;
    }
    return d->currentView()->imageView();
}

void ViewOutPage::updateLayout()
{
    d->mDocumentViewContainer->updateLayout();
}

void ViewOutPage::reset()
{
    d->mDocumentViewController->setView(0);
    d->mDocumentViewContainer->reset();
    d->mDocumentViews.clear();

    Q_FOREACH(Qroilib::DocumentView * view, d->mDocumentViews) {
        delete view;
    }
}

void ViewOutPage::setCurrentView(Qroilib::DocumentView* view)
{
    d->setCurrentView(view);
}

Qroilib::DocumentView* ViewOutPage::currentView() const
{
    if (!d)
        return nullptr;
    return d->currentView();
}

Qroilib::DocumentView* ViewOutPage::view(int n) const
{
    if (d->mDocumentViews.size() <= n)
        return nullptr;
    return d->mDocumentViews[n];
}

IplImage* ViewOutPage::getIplcolor()
{
    Qroilib::DocumentView *v = currentView();
    const QImage *pimg = v->image();
    if (!pimg)
        return nullptr;
    if (pimg->isNull())
        return nullptr;

    cv::Mat frame;
    qimage_to_mat(pimg, frame);

    IplImage riplImg;
    IplImage *iplImg;
    static IplImage *colorImg = nullptr;

    riplImg = frame;
    iplImg = &riplImg;

    CvSize isize = cvSize(iplImg->width, iplImg->height);
    if (colorImg == nullptr)
       colorImg = cvCreateImage(isize, IPL_DEPTH_8U, 3);
    if (iplImg->nChannels == 1)
        cvCvtColor(iplImg, colorImg, CV_GRAY2RGB);
    else if (iplImg->nChannels == 3)
        cvCopy(iplImg, colorImg);
    else if (iplImg->nChannels == 4) {
        if (strncmp(iplImg->channelSeq, "BGRA", 4) == 0)
            cvCvtColor(iplImg, colorImg, CV_BGRA2RGB);
        else
            cvCvtColor(iplImg, colorImg, CV_RGBA2RGB);
    }
    return colorImg;
}

bool ViewOutPage::eventFilter(QObject *obj, QEvent *event)
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

                QString str, str1;
                str.sprintf("x:%d y:%d  r:%d g:%d b:%d", x,y ,r,g,b);

                ViewMainPage*v = theMainWindow->viewMainPage();
                cv::Mat hsv[3];
                cv::split(v->img_hsv, hsv);
                int cx = pimg->width();
                unsigned char *data = hsv[0].data;

                str1.sprintf(" Hue: %d", data[y*cx + x]);
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
