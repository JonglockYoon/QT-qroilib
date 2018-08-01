// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2011 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "documentviewcontainer.h"

// Local
#include <qroilib/documentview/documentview.h>
#include <qroilib/graphicswidgetfloater.h>
#include <qroilib/gvdebug.h>

// Qt
#include <QGraphicsScene>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDebug>
#include <QtMath>
#include <QShortcut>

#include "roiscene.h"

namespace Qroilib
{

typedef QSet<DocumentView*> DocumentViewSet;
typedef QHash<QUrl, DocumentView::Setup> SetupForUrl;

struct DocumentViewContainerPrivate
{
    DocumentViewContainer* q;
    RoiScene* mScene;
    SetupForUrl mSetupForUrl;
    DocumentViewSet mAddedViews;
    DocumentViewSet mRemovedViews;
    QTimer* mLayoutUpdateTimer;

    void scheduleLayoutUpdate()
    {
        mLayoutUpdateTimer->start();
    }

    /**
     * Remove view from set, move it to mRemovedViews so that it is later
     * deleted.
     */
    bool removeFromSet(DocumentView* view, DocumentViewSet* set)
    {
        DocumentViewSet::Iterator it = set->find(view);
        if (it == set->end()) {
            return false;
        }
        set->erase(it);
        mRemovedViews << view;
        scheduleLayoutUpdate();
        return true;
    }

    void resetSet(DocumentViewSet* set)
    {
        qDeleteAll(*set);
        set->clear();
    }
};

DocumentViewContainer::DocumentViewContainer(QWidget* parent)
: QGraphicsView(parent)
, d(new DocumentViewContainerPrivate)
{
    d->q = this;
    d->mScene = new RoiScene(this);

    setScene(d->mScene);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    d->mLayoutUpdateTimer = new QTimer(this);
    d->mLayoutUpdateTimer->setInterval(0);
    d->mLayoutUpdateTimer->setSingleShot(true);
    connect(d->mLayoutUpdateTimer, &QTimer::timeout, this, &DocumentViewContainer::updateLayout);

}

DocumentViewContainer::~DocumentViewContainer()
{
    delete d;
}

DocumentView* DocumentViewContainer::createView(const QUrl url)
{
    DocumentView* view = new DocumentView(d->mScene, url);
    d->mAddedViews << view;
    view->show();
    d->scheduleLayoutUpdate();

    connect(view, SIGNAL(scaleChanged(qreal)), this, SLOT(adjustScale(qreal)));
    connect(view, &DocumentView::scaleChanged, this, &DocumentViewContainer::adjustScale);
    connect(view, SIGNAL(updateLayout()), this, SLOT(resizeUpdateLayout()));

    return view;
}

void DocumentViewContainer::adjustScale(qreal scale)
{
    setTransform(QTransform::fromScale(scale, scale));
}

void DocumentViewContainer::deleteView(DocumentView* view)
{
    d->removeFromSet(view, &d->mAddedViews);
}

//DocumentView::Setup DocumentViewContainer::savedSetup(const QUrl& url) const
//{
//    return d->mSetupForUrl.value(url);
//}

void DocumentViewContainer::updateSetup(DocumentView* view)
{
    d->mSetupForUrl[view->url()] = view->setup();
}

void DocumentViewContainer::reset()
{
    d->resetSet(&d->mAddedViews);
    d->resetSet(&d->mRemovedViews);

    delete d->mScene;
}

void DocumentViewContainer::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updateLayout();
}

void DocumentViewContainer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    d->mScene->setSceneRect(rect());
    updateLayout();
}

static bool viewLessThan(DocumentView* v1, DocumentView* v2)
{
    return v1->sortKey() < v2->sortKey();
}

void DocumentViewContainer::resizeUpdateLayout()
{
    d->mScene->setSceneRect(rect());
    updateLayout();
}

void DocumentViewContainer::updateLayout()
{
    // Stop update timer: this is useful if updateLayout() is called directly
    // and not through scheduleLayoutUpdate()
    d->mLayoutUpdateTimer->stop();
    QList<DocumentView*> views = (d->mAddedViews).toList();
    qSort(views.begin(), views.end(), viewLessThan);
    int count = 0;
    Q_FOREACH(DocumentView * view, views) {
        if (view->isVisible())
            count++;
    }
    if (!views.isEmpty()) {
        // Compute column count
        int colCount;
        switch (count) {
        case 1:
            colCount = 1;
            break;
        case 2:
            colCount = 2;
            break;
        case 3:
            colCount = 3;
            break;
        case 4:
            colCount = 2;
            break;
        case 5:
            colCount = 3;
            break;
        case 6:
            colCount = 3;
            break;
        default:
            colCount = 3;
            break;
        }

        int rowCount = qCeil(count / qreal(colCount));
        Q_ASSERT(rowCount > 0);
        int viewWidth = width() / colCount;
        int viewHeight = height() / rowCount;

        int col = 0;
        int row = 0;

        Q_FOREACH(DocumentView * view, views) {
            if (!view->isVisible())
                continue;
            QRect rect;
            rect.setLeft(col * viewWidth);
            rect.setTop(row * viewHeight);
            rect.setWidth(viewWidth);
            rect.setHeight(viewHeight);

            // Not animated, set final geometry and opacity now
            view->setGeometry(rect);
            view->setOpacity(1);

            ++col;
            if (col == colCount) {
                col = 0;
                ++row;
            }
        }
    }

    // Handle removed views
    Q_FOREACH(DocumentView* view, d->mRemovedViews) {
        view->deleteLater();
    }

    d->mRemovedViews.clear();
}

void DocumentViewContainer::showMessageWidget(QGraphicsWidget* widget, Qt::Alignment align)
{
    DocumentView* view = 0;

    GV_RETURN_IF_FAIL(!d->mAddedViews.isEmpty());
    view = *d->mAddedViews.begin();

    GV_RETURN_IF_FAIL(view);

    widget->setParentItem(view);
    GraphicsWidgetFloater* floater = new GraphicsWidgetFloater(view);
    floater->setChildWidget(widget);
    floater->setAlignment(align);
    widget->show();
    widget->setZValue(1);
}


} // namespace
