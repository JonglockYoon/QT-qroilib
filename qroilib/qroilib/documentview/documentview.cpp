// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <agateau@kde.org>
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

// Qt
#include <QApplication>
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QWeakPointer>
#include <QPointer>
#include <QDebug>
#include <QIcon>
#include <QUrl>
#include <QAction>
#include <QClipboard>
#include <QDir>

// Local
#include <qroilib/document/document.h>
#include <qroilib/document/documentfactory.h>
#include <qroilib/documentview/birdeyeview.h>
#include <qroilib/documentview/messageviewadapter.h>
#include <qroilib/documentview/rasterimageview.h>
#include <qroilib/documentview/rasterimageviewadapter.h>
#include <qroilib/graphicswidgetfloater.h>
#include <qroilib/gvdebug.h>
#include <qroilib/mimetypeutils.h>
#include <qroilib/signalblocker.h>

// Self
#include "documentview.h"
//#include "mainwindow.h"

#include "birdeyeview.h"
#include "savefile.h"

#include "roiscene.h"
#include "object.h"
#include "roirenderer.h"

#include "addremoveroiobject.h"
#include "grouplayer.h"
#include "hexagonalrenderer.h"
#include "isometricrenderer.h"
#include "layeroffsettool.h"
#include "roimap.h"
#include "roiobject.h"
#include "roiobjectmodel.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "objectgroup.h"
#include "abstracttool.h"
#include "toolmanager.h"

#include "roiobjectitem.h"
//#include "recipedata.h"
//#include "common.h"

#include "objectselectiontool.h"
#include "createellipseobjecttool.h"
#include "createobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createpolylineobjecttool.h"
#include "createpointobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createpatternobjecttool.h"
#include "createtextobjecttool.h"

namespace Qroilib
{

#undef ENABLE_LOG
#undef LOG
//#define ENABLE_LOG
#ifdef ENABLE_LOG
#define LOG(x) //qDebug() << x
#else
#define LOG(x) ;
#endif

static const qreal REAL_DELTA = 0.001;
static const qreal MAXIMUM_ZOOM_VALUE = qreal(DocumentView::MaximumZoom);

static const int COMPARE_MARGIN = 4;

const int DocumentView::MaximumZoom = 16;
const int DocumentView::AnimDuration = 250;

struct DocumentViewPrivate
{
    DocumentView* q;
    //int mSortKey; // Used to sort views when displayed in compare mode

    QScopedPointer<AbstractDocumentViewAdapter> mAdapter;
    QList<qreal> mZoomSnapValues;
    Document::Ptr mDocument;
    DocumentView::Setup mSetup;
    bool mCurrent;
    bool mCompareMode;
    bool mEraseBorders;

    void setCurrentAdapter(AbstractDocumentViewAdapter* adapter)
    {
        Q_ASSERT(adapter);
        mAdapter.reset(adapter);

        adapter->widget()->setParentItem(q);
        resizeAdapterWidget();

        if (adapter->canZoom()) {
            QObject::connect(adapter, SIGNAL(zoomChanged(qreal)),
                             q, SLOT(slotZoomChanged(qreal)));
            QObject::connect(adapter, SIGNAL(zoomInRequested(QPointF)),
                             q, SLOT(zoomIn(QPointF)));
            QObject::connect(adapter, SIGNAL(zoomOutRequested(QPointF)),
                             q, SLOT(zoomOut(QPointF)));
            QObject::connect(adapter, SIGNAL(zoomToFitChanged(bool)),
                             q, SIGNAL(zoomToFitChanged(bool)));
        }
        QObject::connect(adapter, SIGNAL(scrollPosChanged()),
                         q, SIGNAL(positionChanged()));

        QObject::connect(adapter, SIGNAL(completed()),
                         q, SLOT(slotCompleted()));

        adapter->loadConfig();

        adapter->widget()->installSceneEventFilter(q);
        if (mCurrent) {
            adapter->widget()->setFocus();
        }

        if (mSetup.valid && adapter->canZoom()) {
            adapter->setZoomToFit(mSetup.zoomToFit);
            if (!mSetup.zoomToFit) {
                adapter->setZoom(mSetup.zoom);
                adapter->setScrollPos(mSetup.position);
            }
        }
        q->adapterChanged();
        q->positionChanged();
        if (adapter->canZoom()) {
            if (adapter->zoomToFit()) {
                q->zoomToFitChanged(true);
            } else {
                q->zoomChanged(adapter->zoom());
            }
        }
        if (adapter->rasterImageView()) {
            //QObject::connect(adapter->rasterImageView(), SIGNAL(currentToolChanged(AbstractTool*)),
            //                 q, SIGNAL(currentToolChanged(AbstractTool*)));
        }
    }


    void setupBirdEyeView()
    {
        if (q->mBirdEyeView) {
            delete q->mBirdEyeView;
        }
        q->mBirdEyeView = new BirdEyeView(q);
        q->mBirdEyeView->setZValue(1);
    }

    void updateCaption()
    {
        QString caption;

        Document::Ptr doc = mAdapter->document();
        if (!doc) {
            //emit q->captionUpdateRequested(caption);
            return;
        }

        caption = doc->url().fileName();
        QSize size = doc->size();
        if (size.isValid()) {
            caption +=
                QString(" - %1x%2")
                .arg(size.width())
                .arg(size.height());
            if (mAdapter->canZoom()) {
                int intZoom = qRound(mAdapter->zoom() * 100);
                caption += QString(" - %1%")
                           .arg(intZoom);
            }
        }
        //emit q->captionUpdateRequested(caption);
    }

    void uncheckZoomToFit()
    {
        if (mAdapter->zoomToFit()) {
            mAdapter->setZoomToFit(false);
        }
    }

    void setZoom(qreal zoom, const QPointF& center = QPointF(-1, -1))
    {
        uncheckZoomToFit();
        zoom = qBound(q->minimumZoom(), zoom, MAXIMUM_ZOOM_VALUE);
        mAdapter->setZoom(zoom, center);
    }

    void updateZoomSnapValues()
    {
        qreal min = q->minimumZoom();

        mZoomSnapValues.clear();
        if (min < 1.) {
            mZoomSnapValues << min;
            for (qreal invZoom = 16.; invZoom > 1.; invZoom /= 2.) {
                qreal zoom = 1. / invZoom;
                if (zoom > min) {
                    mZoomSnapValues << zoom;
                }
            }
        }
        for (qreal zoom = 1; zoom <= MAXIMUM_ZOOM_VALUE ; zoom += 1.) {
            mZoomSnapValues << zoom;
        }

        q->minimumZoomChanged(min);
    }

    void resizeAdapterWidget()
    {
        QRectF rect = QRectF(QPointF(0, 0), q->boundingRect().size());
        if (mCompareMode) {
            rect.adjust(COMPARE_MARGIN, COMPARE_MARGIN, -COMPARE_MARGIN, -COMPARE_MARGIN);
        }
        mAdapter->widget()->setGeometry(rect);
    }

};

DocumentView::DocumentView(RoiScene* scene, const QUrl url)
    :d(new DocumentViewPrivate)
,mRoi(nullptr)
,mRenderer(nullptr)
,mToolManager(nullptr)
,mSelectedTool(nullptr)
,mCurrentObject(nullptr)
,mRoiObjectModel(new RoiObjectModel(this))
,mUndoStack(new QUndoStack(this))
,bMultiView(false)
,seq(-1)
{
    connect(mUndoStack, &QUndoStack::cleanChanged,
            this, &DocumentView::modifiedChanged);

    mRoiObjectModel->setMapDocument(this);

    connect(mRoiObjectModel, SIGNAL(objectsAdded(QList<RoiObject*>)),
            SIGNAL(objectsAdded(QList<RoiObject*>)));
    connect(mRoiObjectModel, SIGNAL(objectsChanged(QList<RoiObject*>)),
            SIGNAL(objectsChanged(QList<RoiObject*>)));
    connect(mRoiObjectModel, SIGNAL(objectsTypeChanged(QList<RoiObject*>)),
            SIGNAL(objectsTypeChanged(QList<RoiObject*>)));
    connect(mRoiObjectModel, SIGNAL(objectsRemoved(QList<RoiObject*>)),
            SLOT(onObjectsRemoved(QList<RoiObject*>)));

    connect(mRoiObjectModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(onRoiObjectModelRowsInserted(QModelIndex,int,int)));
    connect(mRoiObjectModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            SLOT(onRoiObjectModelRowsInsertedOrRemoved(QModelIndex,int,int)));
    connect(mRoiObjectModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            SLOT(onObjectsMoved(QModelIndex,int,int,QModelIndex,int)));

    setFlag(ItemIsFocusable);
    setFlag(ItemIsSelectable);
    setFlag(ItemClipsChildrenToShape);

    d->q = this;
    mBirdEyeView = 0;
    d->mCurrent = false;
    d->mCompareMode = false;
    d->mEraseBorders = false;

    setOpacity(0);

    scene->addItem(this);
    mRoiScene = scene;

    d->setCurrentAdapter(new EmptyAdapter);
    InitRoi(url);
    scene->setMapDocument(this);

}

QString DocumentView::ChannelName(const QUrl url)
{
    QString str = url.toString();

    QStringList tokens = str.split(':');
    QString filename = tokens[1];
    tokens = filename.split('.');
    QString chname = tokens[0];
    return chname;
}

void DocumentView::InitRoi(const QUrl url)
{
    if (mRoi)
        delete mRoi;
    mRoi = new RoiMap(RoiMap::Orthogonal, 640, 480, 1, 1);
    mRoi->setLayerDataFormat(RoiMap::CSV);
    mRoi->setRenderOrder(RoiMap::RightDown);

    if (mRenderer)
        delete mRenderer;
    mRenderer = new OrthogonalRenderer(mRoi);
    mRenderer->setMapDocument(this);

    if (mToolManager)
        delete mToolManager;
    mToolManager = new ToolManager(this);
    mToolManager->setMapDocument(this);

    mToolManager->registerTool(new LayerOffsetTool(this)); // do not remove.

    setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged, this, &DocumentView::setSelectedTool);

    objGroup = new ObjectGroup(ChannelName(url), -1, -1);
    addLayer((Layer *)objGroup);
    setCurrentLayer((Layer *)objGroup);

    objSelTool = new ObjectSelectionTool(this);
    pointObjectsTool = new CreatePointObjectTool(this);
    rectangleObjectsTool = new CreateRectangleObjectTool(this);
    patternroiObjectsTool = new CreatePatternObjectTool(this);
//    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
//    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
//    CreateObjectTool *polylineObjectsTool = new CreatePolylineObjectTool(this);
//    CreateObjectTool *textObjectsTool = new CreateTextObjectTool(this);


    actSelectTool = registerTool(objSelTool);
    actCreatePoint = registerTool(pointObjectsTool);
    actCreateRectangle = registerTool(rectangleObjectsTool);
    actCreatePattern = registerTool(patternroiObjectsTool);
//    act = registerTool(new EditPolygonTool(QAction *));
//    act = registerTool(ellipseObjectsTool);
//    act = registerTool(polygonObjectsTool);
//    act = registerTool(polylineObjectsTool);

    ToolsSelect();
    //selectTool(actSelectTool);
}

const QImage* DocumentView::image()
{
    return (const QImage *)&document()->image();
}

void DocumentView::ToolsSelect()
{


    AbstractTool *tool = actSelectTool->data().value<AbstractTool*>();
    setSelectedTool(tool);

}

DocumentView::~DocumentView()
{
    d->mAdapter.reset();
    delete d;
}

void DocumentView::createAdapterForDocument(int seq)
{
    const MimeTypeUtils::Kind documentKind = d->mDocument->kind();
    if (d->mAdapter && documentKind == d->mAdapter->kind() && documentKind != MimeTypeUtils::KIND_UNKNOWN) {
        // Do not reuse for KIND_UNKNOWN: we may need to change the message
        LOG("Reusing current adapter");
        return;
    }
    AbstractDocumentViewAdapter* adapter = 0;
    switch (documentKind) {
    case MimeTypeUtils::KIND_RASTER_IMAGE:
        adapter = new RasterImageViewAdapter(seq);

        QObject::connect(adapter->rasterImageView(), SIGNAL(finishLoadDocument()), this, SLOT(finishLoadDocument()));

        break;
    case MimeTypeUtils::KIND_UNKNOWN:
        adapter = new MessageViewAdapter;
        static_cast<MessageViewAdapter*>(adapter)->setErrorMessage(tr("Qroilib does not know how to display this kind of document"));
        break;
    default:
        qWarning() << "should not be called for documentKind=" << documentKind;
        adapter = new MessageViewAdapter;
        break;
    }

    d->setCurrentAdapter(adapter);
}


void DocumentView::finishLoadDocument()
{

}

void DocumentView::openUrl(const QUrl &url, const DocumentView::Setup& setup, int seq)
{
    d->mSetup = setup;
    d->mDocument = DocumentFactory::instance()->load(url);

    finishOpenUrl(seq);
    d->setupBirdEyeView();
}

void DocumentView::finishOpenUrl(int seq)
{
    disconnect(d->mDocument.data(), SIGNAL(kindDetermined(QUrl)),
               this, SLOT(finishOpenUrl(int)));
    GV_RETURN_IF_FAIL(d->mDocument->loadingState() >= Document::KindDetermined);

    createAdapterForDocument(seq);

    d->mAdapter->setDocument(d->mDocument);
    d->updateCaption();

    RoiScene *mapScene = mRoiScene;
    mapScene->setSelectedTool(mSelectedTool);
    mapScene->enableSelectedTool();
    if (mSelectedTool)
        setCursor(mSelectedTool->cursor());
    else
        unsetCursor();

    emit completed(seq);
}

void DocumentView::loadAdapterConfig()
{
    d->mAdapter->loadConfig();
}

RasterImageView* DocumentView::imageView() const
{
    return d->mAdapter->rasterImageView();
}

void DocumentView::slotCompleted()
{
    d->updateCaption();
    d->updateZoomSnapValues();
    if (!d->mAdapter->zoomToFit()) {
        qreal min = minimumZoom();
        if (d->mAdapter->zoom() < min) {
            d->mAdapter->setZoom(min);
        }
    }
}

DocumentView::Setup DocumentView::setup()
{
    Setup setup;
    if (d->mAdapter->canZoom()) {
        setup.valid = true;
        setup.zoomToFit = zoomToFit();
        if (!setup.zoomToFit) {
            setup.zoom = zoom();
            setup.position = position();
        }
    }
    return setup;
}

bool DocumentView::canZoom() const
{
    return d->mAdapter->canZoom();
}

void DocumentView::setZoomToFit(bool on)
{
    if (on == d->mAdapter->zoomToFit()) {
        return;
    }
    d->mAdapter->setZoomToFit(on);
    if (!on) {
        d->mAdapter->setZoom(1.);
    }
    emit updateLayout();
}

bool DocumentView::zoomToFit() const
{
    bool b = d->mAdapter->zoomToFit();
    return b;
}

void DocumentView::zoomActualSize()
{
    d->uncheckZoomToFit();
    d->mAdapter->setZoom(1.);
    emit updateLayout();

}

void DocumentView::zoomIn(const QPointF& center)
{
    if (bMultiView) // 두개이상 view일때는 zoom in 막음.
        return;
    qreal currentZoom = d->mAdapter->zoom();

    Q_FOREACH(qreal zoom, d->mZoomSnapValues) {
        if (zoom > currentZoom + REAL_DELTA) {
            d->setZoom(zoom, center);
            emit updateLayout();
            return;
        }
    }
}

void DocumentView::zoomOut(const QPointF& center)
{
    qreal currentZoom = d->mAdapter->zoom();

    QListIterator<qreal> it(d->mZoomSnapValues);
    it.toBack();
    while (it.hasPrevious()) {
        qreal zoom = it.previous();
        if (zoom < currentZoom - REAL_DELTA) {
            d->setZoom(zoom, center);
            emit updateLayout();
            return;
        }
    }
}

void DocumentView::slotZoomChanged(qreal zoom)
{
    d->updateCaption();
    zoomChanged(zoom);

    d->resizeAdapterWidget();
    if (mBirdEyeView) {
        mBirdEyeView->slotZoomOrSizeChanged();
    }

    //emit mapChanged();
    emit selectedObjectsChanged();

    emit updateLayout();
    //RasterImageView* pView = imageView();
    //pView->updateBuffer();

    //mRoiScene->updateSceneRect();

    mRoiScene->resizeAllObjectItems();
    //qDebug() << "DocumentView::slotZoomChanged() selectedObjectsChanged()";
}

void DocumentView::setZoom(qreal zoom)
{
    d->setZoom(zoom);

    d->resizeAdapterWidget();
    if (mBirdEyeView) {
        mBirdEyeView->slotZoomOrSizeChanged();
    }
    emit updateLayout();

}

qreal DocumentView::zoom() const
{
    const qreal r = d->mAdapter->zoom();
    return r;
}

void DocumentView::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    if (d->mAdapter->canZoom() && event->modifiers() & Qt::ControlModifier) {
        // Ctrl + wheel => zoom in or out
        if (event->delta() > 0) {
            zoomIn(event->pos());
        } else {
            zoomOut(event->pos());
        }
        return;
    }
    if (event->modifiers() == Qt::NoModifier) {
        // Browse with mouse wheel
        if (event->delta() > 0) {
            //previousImageRequested();
        } else {
            //nextImageRequested();
        }
        return;
    }
    // Scroll
    qreal dx = 0;
    // 16 = pixels for one line
    // 120: see QWheelEvent::delta() doc
    qreal dy = -qApp->wheelScrollLines() * 16 * event->delta() / 120;
    if (event->orientation() == Qt::Horizontal) {
        qSwap(dx, dy);
    }
    d->mAdapter->setScrollPos(d->mAdapter->scrollPos() + QPointF(dx, dy));
}

void DocumentView::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    // Filter out context menu if Ctrl is down to avoid showing it when
    // zooming out with Ctrl + Right button
    if (event->modifiers() != Qt::ControlModifier) {
        contextMenuRequested();
    }
}

void DocumentView::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    QRectF visibleRect = mapRectFromItem(d->mAdapter->widget(), d->mAdapter->visibleDocumentRect());
    if (d->mEraseBorders) {
        QRegion borders = QRegion(boundingRect().toRect())
            - QRegion(visibleRect.toRect());
        Q_FOREACH(const QRect& rect, borders.rects()) {
            painter->eraseRect(rect);
        }
    }

    if (d->mCompareMode && d->mCurrent) {
        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(palette().highlight().color(), 2));
        painter->setRenderHint(QPainter::Antialiasing);
        QRectF selectionRect = visibleRect.adjusted(-2, -2, 2, 2);
        painter->drawRoundedRect(selectionRect, 3, 3);
        painter->restore();
    }
}



qreal DocumentView::minimumZoom() const
{
    // There is no point zooming out less than zoomToFit, but make sure it does
    // not get too small either
    return qBound(qreal(0.001), d->mAdapter->computeZoomToFit(), qreal(1.));
}

void DocumentView::setCompareMode(bool compare)
{
    d->mCompareMode = compare;
    d->resizeAdapterWidget();
}

void DocumentView::setCurrent(bool value)
{
    d->mCurrent = value;
    if (value) {
        d->mAdapter->widget()->setFocus();
    }
    update();
}

bool DocumentView::isCurrent() const
{
    return d->mCurrent;
}

QPoint DocumentView::position()
{
    QPoint pt = d->mAdapter->scrollPos().toPoint();
    emit selectedObjectsChanged();
    return  pt;
}

void DocumentView::setPosition(const QPoint& pos)
{
    d->mAdapter->setScrollPos(pos);
}

Document::Ptr DocumentView::document() const
{
    return d->mDocument;
}

QUrl DocumentView::url() const
{
    Document::Ptr doc = d->mDocument;
    return doc ? doc->url() : QUrl();
}

void DocumentView::emitFocused()
{
    focused(this);
}

void DocumentView::setGeometry(const QRectF& rect)
{
    QGraphicsWidget::setGeometry(rect);

    d->resizeAdapterWidget();
    if (mBirdEyeView) {
        mBirdEyeView->slotZoomOrSizeChanged();
    }

    emit mapChanged();
    emit selectedObjectsChanged();
    //qDebug() << "DocumentView::setGeometry() selectedObjectsChanged()";
}

void DocumentView::moveTo(const QRect& rect)
{
    setGeometry(rect);
}

bool DocumentView::sceneEventFilter(QGraphicsItem*, QEvent* event)
{
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        QMetaObject::invokeMethod(this, "emitFocused", Qt::QueuedConnection);
    } else if (event->type() == QEvent::GraphicsSceneHoverMove) {
        if (mBirdEyeView) {
            mBirdEyeView->onMouseMoved();
        }
    }
    return false;
}

int DocumentView::sortKey() const
{
    return seq;
//    for (Layer *layer : mRoi->layers()) {
//        return layer->name();
//    }
//    return QString();
}

void DocumentView::setEraseBorders(bool value)
{
    d->mEraseBorders = value;
}

void DocumentView::hideAndDeleteLater()
{
    hide();
    deleteLater();
}

void DocumentView::setSelectedArea(const QRegion &selection)
{
    if (mSelectedArea != selection) {
        const QRegion oldSelectedArea = mSelectedArea;
        mSelectedArea = selection;
        emit selectedAreaChanged(mSelectedArea, oldSelectedArea);
    }
}

void DocumentView::setSelectedObjects(const QList<RoiObject *> &selectedObjects)
{
    mSelectedObjects = selectedObjects;
    emit selectedObjectsChanged();

    ObjectGroup *singleObjectGroup = nullptr;
    for (RoiObject *object : selectedObjects) {
        ObjectGroup *currentObjectGroup = object->objectGroup();

        if (!singleObjectGroup) {
            singleObjectGroup = currentObjectGroup;
        } else if (singleObjectGroup != currentObjectGroup) {
            singleObjectGroup = nullptr;
            break;
        }
    }

    // Switch the current object layer if only one object layer (and/or its objects)
    // are included in the current selection.
    if (singleObjectGroup)
        setCurrentLayer(singleObjectGroup);

    if (selectedObjects.size() == 1) {
        setCurrentObject(selectedObjects.first());
    }
    //qDebug() << "DocumentView::setSelectedObjects() selectedObjectsChanged()";
}

void DocumentView::setCurrentObject(Object *object)
{
    if (object == mCurrentObject)
        return;

    mCurrentObject = object;
}

QList<Object *> DocumentView::currentObjects() const
{
    QList<Object*> objects;
    if (mCurrentObject)
        objects.append(mCurrentObject);
    return objects;
}


void DocumentView::setProperty(Object *object,
                           const QString &name,
                           const QVariant &value)
{
    const bool hadProperty = object->hasProperty(name);

    object->setProperty(name, value);

    if (hadProperty)
        emit propertyChanged(object, name);
    else
        emit propertyAdded(object, name);
}

void DocumentView::setProperties(Object *object, const Properties &properties)
{
    object->setProperties(properties);
    emit propertiesChanged(object);
}

void DocumentView::removeProperty(Object *object, const QString &name)
{
    object->removeProperty(name);
    emit propertyRemoved(object, name);
}



void DocumentView::onRoiObjectModelRowsInserted(const QModelIndex &parent,
                                               int first, int last)
{
    ObjectGroup *objectGroup = mRoiObjectModel->toObjectGroup(parent);
    if (!objectGroup) // we're not dealing with insertion of objects
        return;

    emit objectsInserted(objectGroup, first, last);
    onRoiObjectModelRowsInsertedOrRemoved(parent, first, last);
}

void DocumentView::onRoiObjectModelRowsInsertedOrRemoved(const QModelIndex &parent,
                                                        int first, int last)
{
    Q_UNUSED(first)

    ObjectGroup *objectGroup = mRoiObjectModel->toObjectGroup(parent);
    if (!objectGroup)
        return;

    // Inserting or removing objects changes the index of any that come after
    const int lastIndex = objectGroup->objectCount() - 1;
    if (last < lastIndex)
        emit objectsIndexChanged(objectGroup, last + 1, lastIndex);
}


void DocumentView::onObjectsMoved(const QModelIndex &parent, int start, int end,
                                 const QModelIndex &destination, int row)
{
    if (parent != destination)
        return;

    ObjectGroup *objectGroup = mRoiObjectModel->toObjectGroup(parent);

    // Determine the full range over which object indexes changed
    const int first = qMin(start, row);
    const int last = qMax(end, row - 1);

    emit objectsIndexChanged(objectGroup, first, last);
}

void DocumentView::onLayerAdded(Layer *layer)
{
    emit layerAdded(layer);

    // Select the first layer that gets added to the roimap
    if (mRoi->layerCount() == 1 && mRoi->layerAt(0) == layer)
        setCurrentLayer(layer);
}


/**
 * Before forwarding the signal, the objects are removed from the list of
 * selected objects, triggering a selectedObjectsChanged signal when
 * appropriate.
 */
void DocumentView::onObjectsRemoved(const QList<RoiObject*> &objects)
{
    deselectObjects(objects);
    emit objectsRemoved(objects);
}

static void collectObjects(Layer *layer, QList<RoiObject*> &objects)
{
    switch (layer->layerType()) {
    case Layer::ObjectGroupType:
        objects.append(static_cast<ObjectGroup*>(layer)->objects());
        break;
    case Layer::GroupLayerType:
        for (auto childLayer : *static_cast<GroupLayer*>(layer))
            collectObjects(childLayer, objects);
        break;

    }
}

void DocumentView::onLayerAboutToBeRemoved(GroupLayer *groupLayer, int index)
{
    Layer *layer = groupLayer ? groupLayer->layerAt(index) : mRoi->layerAt(index);
    if (layer == mCurrentObject)
        setCurrentObject(nullptr);

    // Deselect any objects on this layer when necessary
    if (layer->isObjectGroup() || layer->isGroupLayer()) {
        QList<RoiObject*> objects;
        collectObjects(layer, objects);
        deselectObjects(objects);
    }

    emit layerAboutToBeRemoved(groupLayer, index);
}

void DocumentView::onLayerRemoved(Layer *layer)
{
    if (mCurrentLayer && mCurrentLayer->isParentOrSelf(layer))
        setCurrentLayer(nullptr);

    emit layerRemoved(layer);
}


void DocumentView::deselectObjects(const QList<RoiObject *> &objects)
{
    // Unset the current object when it was part of this list of objects
    if (mCurrentObject && mCurrentObject->typeId() == Object::RoiObjectType)
        if (objects.contains(static_cast<RoiObject*>(mCurrentObject)))
            setCurrentObject(nullptr);

    int removedCount = 0;
    for (RoiObject *object : objects)
        removedCount += mSelectedObjects.removeAll(object);

    if (removedCount > 0)
        emit selectedObjectsChanged();
}

void DocumentView::duplicateObjects(const QList<RoiObject *> &objects)
{
    if (objects.isEmpty())
        return;

    mUndoStack->beginMacro(tr("Duplicate %n Object(s)", "", objects.size()));

    QList<RoiObject*> clones;
    for (const RoiObject *roiObject : objects) {
        RoiObject *clone = roiObject->clone();
        clone->resetId();
        clones.append(clone);
        mUndoStack->push(new AddRoiObject(this,
                                          roiObject->objectGroup(),
                                          clone));
        AddRoiObject(this, roiObject->objectGroup(), clone);
    }

    mUndoStack->endMacro();
    setSelectedObjects(clones);
}

void DocumentView::removeObjects(const QList<RoiObject *> &objects)
{
    if (objects.isEmpty())
        return;

    mUndoStack->beginMacro(tr("Remove %n Object(s)", "", objects.size()));
    const auto objectsCopy = objects;   // original list may get modified
    for (RoiObject *roiObject : objectsCopy) {
        if (roiObject->mParent != nullptr && roiObject->mParent->objectGroup() != nullptr)
            mUndoStack->push(new RemoveRoiObject(this, roiObject->mParent));
        if (roiObject->mPattern != nullptr && roiObject->mPattern->objectGroup() != nullptr)
            mUndoStack->push(new RemoveRoiObject(this, roiObject->mPattern));

        if (roiObject->objectGroup() != nullptr && roiObject->objectGroup() != nullptr)
            mUndoStack->push(new RemoveRoiObject(this, roiObject));
    }
    mUndoStack->endMacro();
}


void DocumentView::createRenderer()
{
    if (mRenderer)
        delete mRenderer;

    switch (mRoi->orientation()) {
    case RoiMap::Isometric:
        mRenderer = new IsometricRenderer(mRoi);
        break;
    case RoiMap::Hexagonal:
        mRenderer = new HexagonalRenderer(mRoi);
        break;
    default:
        mRenderer = new OrthogonalRenderer(mRoi);
        break;
    }
}


void DocumentView::setCurrentLayer(Layer *layer)
{
    if (mCurrentLayer == layer)
        return;

    mCurrentLayer = layer;
    emit currentLayerChanged(mCurrentLayer);

    if (mCurrentLayer)
        if (!mCurrentObject || mCurrentObject->typeId() == Object::LayerType)
            setCurrentObject(mCurrentLayer);
}

void DocumentView::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    if (mSelectedTool) {
        disconnect(mSelectedTool, &AbstractTool::cursorChanged,
                   this, &DocumentView::cursorChanged);
    }

    mSelectedTool = tool;

    //if (mViewWithTool)
    {
        RoiScene *mapScene = mRoiScene;
        mapScene->disableSelectedTool();

        if (tool) {
            mapScene->setSelectedTool(tool);
            mapScene->enableSelectedTool();
        }

        if (tool)
            setCursor(tool->cursor());
        else
            unsetCursor();
    }

    if (tool) {
        connect(tool, &AbstractTool::cursorChanged,
                this, &DocumentView::cursorChanged);
    }
}

void DocumentView::cursorChanged(const QCursor &cursor)
{
     setCursor(cursor);
}

void DocumentView::delete_()
{
    removeObjects(mSelectedObjects);
    selectNone();
}

QAction *DocumentView::registerTool(AbstractTool *tool)
{
    return mToolManager->registerTool(tool);
}

void DocumentView::selectNone()
{
    setSelectedObjects(QList<RoiObject*>());
}

void DocumentView::selectTool(QAction* act)
{
    if (bMultiView) {
        selectNone();
        return;
    }
    if (act == nullptr) {
        mToolManager->setSelectedTool(nullptr);
        return;
    }
    AbstractTool *tool = act->data().value<AbstractTool*>();
    //Qroilib::DocumentView* v = tool->mapDocument();
    mToolManager->setSelectedTool(tool);
}

void DocumentView::clearSelectedObjectItems()
{
    mRoiScene->setSelectedObjectItems(QSet<RoiObjectItem*>());
}

void DocumentView::addLayer(Layer *layer)
{
    roimap()->addLayer((Layer *)layer);
    mRoiScene->setMapDocument(this);
}

} // namespace
