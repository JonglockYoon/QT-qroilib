// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2011 Aurélien Gâteau <agateau@kde.org>
Qroilib : QT ROI Lib
Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

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
#include "documentviewcontroller.h"

// Local
#include "abstractdocumentviewadapter.h"
#include "documentview.h"
#include <qroilib/signalblocker.h>
#include <qroilib/zoomwidget.h>

// Qt
#include <QAction>
#include <QHBoxLayout>
#include <QDebug>
#include <QAction>
#include <QUndoStack>

#include "abstracttool.h"
#include "layer.h"
#include "addremoveroiobject.h"

namespace Qroilib
{

/**
 * A simple container which:
 * - Horizontally center the tool widget
 * - Provide a darker background
 */
class ToolContainerContent : public QWidget
{
public:
    ToolContainerContent(QWidget* parent = 0)
    : QWidget(parent)
    , mLayout(new QHBoxLayout(this))
    {
        mLayout->setMargin(0);
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, pal.color(QPalette::Window).dark(120));
        setPalette(pal);
    }

    void setToolWidget(QWidget* widget)
    {
        mLayout->addWidget(widget, 0, Qt::AlignCenter);
        setFixedHeight(widget->sizeHint().height());
    }

private:
    QHBoxLayout* mLayout;
};

struct DocumentViewControllerPrivate
{
    DocumentViewController* q;
//    KActionCollection* mActionCollection;
    DocumentView* mView;
    ZoomWidget* mZoomWidget;

    QAction * mZoomToFitAction;
    QAction * mActualSizeAction;
    QAction * mZoomInAction;
    QAction * mZoomOutAction;
    QList<QAction *> mActions;

    void setupActions()
    {
        mZoomToFitAction = new QAction(q);//view->addAction("view_zoom_to_fit");
        //view->collection()->setDefaultShortcut(mZoomToFitAction, Qt::Key_F);
        mZoomToFitAction->setCheckable(true);
        mZoomToFitAction->setChecked(true);
        //mZoomToFitAction->setText(tr("Zoom to Fit"));
        //mZoomToFitAction->setIcon(QIcon::fromTheme("zoom-fit-best"));
        mZoomToFitAction->setIconText(QObject::tr("Fit"));
        mZoomToFitAction->setText(QObject::tr("Fit:button Zoom to fit, shown in status bar, keep it short please"));

        mActualSizeAction = new QAction(q);//view->addAction(KStandardAction::ActualSize);
        //mActualSizeAction->setIcon(QIcon::fromTheme("zoom-original"));
        //mActualSizeAction->setText(tr("100%"));
        mActualSizeAction->setIconText(QObject::tr("100%"));
        mActualSizeAction->setText(QObject::tr("100%:button Zoom to original size, shown in status bar, keep it short please"));

        mZoomInAction = new QAction(q);//view->addAction(KStandardAction::ZoomIn);
        mZoomInAction->setIconText(QObject::tr("+"));
        mZoomInAction->setText(QObject::tr("+:button ZoomIn, shown in status bar, keep it short please"));
        mZoomOutAction = new QAction(q);//view->addAction(KStandardAction::ZoomOut);
        mZoomOutAction->setIconText(QObject::tr("-"));
        mZoomOutAction->setText(QObject::tr("-:button ZoomOut, shown in status bar, keep it short please"));

        mActions << mZoomToFitAction << mActualSizeAction << mZoomInAction << mZoomOutAction;
    }

    void connectZoomWidget()
    {
        if (!mZoomWidget || !mView) {
            return;
        }

        QObject::connect(mZoomWidget, &ZoomWidget::zoomChanged, mView, &DocumentView::setZoom);

        QObject::connect(mView, &DocumentView::minimumZoomChanged, mZoomWidget, &ZoomWidget::setMinimumZoom);
        QObject::connect(mView, &DocumentView::zoomChanged, mZoomWidget, &ZoomWidget::setZoom);

        mZoomWidget->setMinimumZoom(mView->minimumZoom());
        mZoomWidget->setZoom(mView->zoom());
    }

    void updateZoomWidgetVisibility()
    {
        if (!mZoomWidget) {
            return;
        }
        mZoomWidget->setVisible(mView && mView->canZoom());
    }

    void updateActions()
    {
        const bool enabled = mView && mView->isVisible() && mView->canZoom();
        Q_FOREACH(QAction * action, mActions) {
            action->setEnabled(enabled);
        }
    }
};

DocumentViewController::DocumentViewController(QObject* parent)
: QObject(parent)
, d(new DocumentViewControllerPrivate)
{
    d->q = this;
    d->mView = 0;
    d->mZoomWidget = 0;

    d->setupActions();
}

DocumentViewController::~DocumentViewController()
{
    delete d;
}

void DocumentViewController::setView(DocumentView* view)
{
    // Forget old view
    if (d->mView) {
        disconnect(d->mView, 0, this, 0);
        Q_FOREACH(QAction * action, d->mActions) {
            disconnect(action, 0, d->mView, 0);
        }
        disconnect(d->mZoomWidget, 0, d->mView, 0);
    }

    // Connect new view
    d->mView = view;
    if (!d->mView) {
        return;
    }

    connect(d->mView, &DocumentView::adapterChanged, this, &DocumentViewController::slotAdapterChanged);
    connect(d->mView, &DocumentView::zoomToFitChanged, this, &DocumentViewController::updateZoomToFitActionFromView);

    connect(d->mZoomToFitAction, SIGNAL(toggled(bool)),
            d->mView, SLOT(setZoomToFit(bool)));
    connect(d->mActualSizeAction, SIGNAL(triggered()),
            d->mView, SLOT(zoomActualSize()));
    connect(d->mZoomInAction, SIGNAL(triggered()),
            d->mView, SLOT(zoomIn()));
    connect(d->mZoomOutAction, SIGNAL(triggered()),
            d->mView, SLOT(zoomOut()));

    d->updateActions();
    updateZoomToFitActionFromView();

    // Sync zoom widget
    d->connectZoomWidget();
    d->updateZoomWidgetVisibility();
}

DocumentView* DocumentViewController::view() const
{
    return d->mView;
}

void DocumentViewController::setZoomWidget(ZoomWidget* widget)
{
    d->mZoomWidget = widget;

    d->mZoomWidget->setActions(
        d->mZoomToFitAction,
        d->mActualSizeAction,
        d->mZoomInAction,
        d->mZoomOutAction
    );

    d->mZoomWidget->setMaximumZoom(qreal(DocumentView::MaximumZoom));

    d->connectZoomWidget();
    d->updateZoomWidgetVisibility();
}

ZoomWidget* DocumentViewController::zoomWidget() const
{
    return d->mZoomWidget;
}

void DocumentViewController::slotAdapterChanged()
{
    d->updateActions();
    d->updateZoomWidgetVisibility();
}

void DocumentViewController::updateZoomToFitActionFromView()
{
    SignalBlocker blocker(d->mZoomToFitAction);
    d->mZoomToFitAction->setChecked(d->mView->zoomToFit());
}


} // namespace
