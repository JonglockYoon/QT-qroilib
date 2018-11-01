/*
viewmainpage.cpp
*/
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

void ViewMainPage::showContextMenu()
{
}

//QSize ViewMainPage::sizeHint() const
//{
//    return QSize(640, 480);
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

Qroilib::DocumentView* ViewMainPage::view(int n) const
{
    if (d->mDocumentViews.size() <= n)
        return nullptr;
    return d->mDocumentViews[n];
}
