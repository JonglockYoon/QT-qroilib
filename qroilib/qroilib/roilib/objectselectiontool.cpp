/*
 * objectselectiontool.cpp
 * Copyright 2010-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "objectselectiontool.h"

#include "changepolygon.h"
#include "layer.h"
#include "roimap.h"
#include "roiobject.h"
#include "roiobjectitem.h"
#include "roiobjectmodel.h"
#include "roirenderer.h"
#include "roiscene.h"
#include "moveroiobject.h"
#include "objectgroup.h"
#include "resizeroiobject.h"
#include "rotateroiobject.h"
#include "selectionrectangle.h"
#include "snaphelper.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

#include <QApplication>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QTransform>
#include <QUndoStack>
#include <QMenu>
#include <QDebug>

#include <cmath>

// MSVC 2010 math header does not come with M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace Qroilib;

namespace Qroilib {

enum AnchorPosition {
    TopLeftAnchor,
    TopRightAnchor,
    BottomLeftAnchor,
    BottomRightAnchor,

    TopAnchor,
    LeftAnchor,
    RightAnchor,
    BottomAnchor,

    CornerAnchorCount = 4,
    AnchorCount = 8,
};

static QPainterPath createResizeArrow(bool straight)
{
    const qreal arrowLength = straight ? 14 : 16;
    const qreal arrowHeadLength = 4.5;
    const qreal arrowHeadWidth = 5;
    const qreal bodyWidth = 1.5;

    QPainterPath path;
    path.lineTo(arrowHeadWidth, arrowHeadLength);
    path.lineTo(0 + bodyWidth, arrowHeadLength);
    path.lineTo(0 + bodyWidth, arrowLength - arrowHeadLength);
    path.lineTo(arrowHeadWidth, arrowLength - arrowHeadLength);
    path.lineTo(0, arrowLength);
    path.lineTo(-arrowHeadWidth, arrowLength - arrowHeadLength);
    path.lineTo(0 - bodyWidth, arrowLength - arrowHeadLength);
    path.lineTo(0 - bodyWidth, arrowHeadLength);
    path.lineTo(-arrowHeadWidth, arrowHeadLength);
    path.closeSubpath();
    path.translate(0, straight ? 2 : 3);

    return path;
}


/**
 * Shared superclass for rotation and resizing handles.
 */
class Handle : public QGraphicsItem
{
public:
    Handle(QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mUnderMouse(false)
    {
        setFlags(QGraphicsItem::ItemIgnoresTransformations |
                 QGraphicsItem::ItemIgnoresParentOpacity);
    }

    void setUnderMouse(bool underMouse)
    {
        if (mUnderMouse != underMouse) {
            mUnderMouse = underMouse;
            update();
        }
    }

protected:
    bool mUnderMouse;
};


/**
 * Rotation origin indicator.
 */
class OriginIndicator : public Handle
{
    DocumentView *mRoiDocument = nullptr;
public:
    OriginIndicator(QGraphicsItem *parent = nullptr)
        : Handle(parent)
    {
        setZValue(10000 + 1);
    }

    QRectF boundingRect() const override
    {
        return QRectF();
        //return QRectF(-9, -9, 18, 18);
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
    DocumentView *mapDocument() const { return mRoiDocument; }
    void setMapDocument(DocumentView *mapDocument) { mRoiDocument = mapDocument; }
};

void OriginIndicator::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *,
                            QWidget *)
{
    static const QLine lines[] = {
        QLine(-8,0, 8,0),
        QLine(0,-8, 0,8),
    };

    if (!mapDocument())
        return;
    QPointF p = pos();
    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();
    double bx = (p.x())*scale - p.x();
    double by = (p.y())*scale - p.y();
    double sx = scroll.x()*scale - scroll.x();
    double sy = scroll.y()*scale - scroll.y();
    double ox = offset.x()*scale - offset.x();
    double oy = offset.y()*scale - offset.y();

    painter->translate(bx+sx-ox, by+sy-oy);
    //painter->scale(defaultDpiScale(), defaultDpiScale());
    painter->setPen(QPen(mUnderMouse ? Qt::white : Qt::lightGray, 1, Qt::DashLine));
    painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));
    painter->translate(1, 1);
    painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
    painter->drawLines(lines, sizeof(lines) / sizeof(lines[0]));
}

/**
 * A resize handle that allows resizing of roimap objects.
 */
class ResizeHandle : public Handle
{
public:
    ResizeHandle(AnchorPosition anchorPosition, QGraphicsItem *parent = nullptr)
        : Handle(parent)
        , mRoiDocument(nullptr)
        , mAnchorPosition(anchorPosition)
        , mResizingLimitHorizontal(false)
        , mResizingLimitVertical(false)
        , mArrow(createResizeArrow(anchorPosition > BottomRightAnchor))
    {
        // The bottom right anchor takes precedence
        setZValue(10000 + 1 + (anchorPosition < TopAnchor ? anchorPosition + 1 : 0));

        QTransform transform;

        switch (anchorPosition) {
        case TopLeftAnchor:     transform.rotate(135);  break;
        case TopRightAnchor:    transform.rotate(-135); break;
        case BottomLeftAnchor:  transform.rotate(45);   break;
        case BottomRightAnchor: transform.rotate(-45);  break;
        case TopAnchor:         transform.rotate(180);  mResizingLimitHorizontal = true; break;
        case LeftAnchor:        transform.rotate(90);   mResizingLimitVertical = true; break;
        case RightAnchor:       transform.rotate(-90);  mResizingLimitVertical = true; break;
        case BottomAnchor:
        default:                mResizingLimitHorizontal = true; break;
        }

        mArrow = transform.map(mArrow);
    }

    AnchorPosition anchorPosition() const { return mAnchorPosition; }
    
    void setResizingOrigin(QPointF resizingOrigin) { mResizingOrigin = resizingOrigin; }
    QPointF resizingOrigin() const { return mResizingOrigin; }
    
    bool resizingLimitHorizontal() const { return mResizingLimitHorizontal; }
    bool resizingLimitVertical() const { return mResizingLimitVertical; }

    QRectF boundingRect() const override
    {
        if (mapDocument() == nullptr)
            return QRectF();

        QRectF b = dpiScaled(mArrow.boundingRect().adjusted(-1, -1, 1, 1));

        RasterImageView *pView = mapDocument()->imageView();
        QPointF offset = pView->imageOffset();
        QPointF scroll = pView->scrollPos();
        const qreal scale = mapDocument()->zoom();

        QPointF p = pos();
        QPointF bp = p*scale - p;
        QPointF bs = scroll*scale - scroll;
        QPointF bo = offset*scale - offset;

        QRectF rect;
        rect.setTopLeft(b.topLeft() + bp + bs - offset);
        rect.setWidth(b.width());
        rect.setHeight(b.height());
        return rect.normalized();
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    DocumentView *mapDocument() const { return mRoiDocument; }
    void setMapDocument(DocumentView *mapDocument) { mRoiDocument = mapDocument; }

private:
    DocumentView *mRoiDocument = nullptr;
    AnchorPosition mAnchorPosition;
    QPointF mResizingOrigin;
    bool mResizingLimitHorizontal;
    bool mResizingLimitVertical;
    QPainterPath mArrow;
};

void ResizeHandle::paint(QPainter *painter,
                         const QStyleOptionGraphicsItem *,
                         QWidget *)
{

    if (!mapDocument())
        return;
    QPen pen(mUnderMouse ? Qt::black : Qt::lightGray, 1);
    QColor brush(mUnderMouse ? Qt::white : Qt::black);

    //painter->scale(defaultDpiScale(), defaultDpiScale());
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(pen);
    painter->setBrush(brush);

    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();

    QPointF p = pos();
    QPainterPath path;
    path.addPath(mArrow);

    double bx = (p.x())*scale - p.x();
    double by = (p.y())*scale - p.y();
    double sx = scroll.x()*scale - scroll.x();
    double sy = scroll.y()*scale - scroll.y();
    double ox = offset.x()*scale - offset.x();
    double oy = offset.y()*scale - offset.y();

    int cnt = path.elementCount();
    for (int i=0; i<cnt; i++)
    {
        QPainterPath::Element e = path.elementAt(i);
        qreal x = bx + e.x + sx;// - ox;
        qreal y = by + e.y + sy;// - oy;
        path.setElementPositionAt(i, x, y);
    }

    painter->drawPath(path);
}

} // namespace Qroilib


ObjectSelectionTool::ObjectSelectionTool(QObject *parent)
    : AbstractObjectTool(tr("Select Objects"),
          QIcon(QLatin1String(":images/22x22/tool-select-objects.png")),
          QKeySequence(tr("S")),
          parent)
    , mSelectionRectangle(new SelectionRectangle)
    , mOriginIndicator(new OriginIndicator)
    //, mTmpOriginIndicator(new OriginIndicator)
    , mMousePressed(false)
    , mHoveredObjectItem(nullptr)
    , mHoveredHandle(nullptr)
    , mClickedObjectItem(nullptr)
    , mClickedOriginIndicator(nullptr)
    , mClickedResizeHandle(nullptr)
    , mResizingLimitHorizontal(false)
    , mResizingLimitVertical(false)
    , mMode(Resize)
    , mAction(NoAction)
{
    mSelectionRectangle->setMapDocument((DocumentView *)parent);

    for (int i = 0; i < AnchorCount; ++i) {
        mResizeHandles[i] = new ResizeHandle(static_cast<AnchorPosition>(i));
        //mTmpResizeHandles[i] = new ResizeHandle(static_cast<AnchorPosition>(i));
    }
}

ObjectSelectionTool::~ObjectSelectionTool()
{
    delete mSelectionRectangle;
    delete mOriginIndicator;

    for (ResizeHandle *handle : mResizeHandles)
        delete handle;
}

void ObjectSelectionTool::activate(RoiScene *scene)
{
    AbstractObjectTool::activate(scene);

    updateHandles();

    connect(mapDocument(), SIGNAL(objectsChanged(QList<RoiObject*>)),
            this, SLOT(updateHandles()));
    connect(mapDocument(), SIGNAL(mapChanged()),
            this, SLOT(updateHandles()));
    connect(scene, SIGNAL(selectedObjectItemsChanged()),
            this, SLOT(updateHandles()));

    connect(mapDocument(), SIGNAL(objectsRemoved(QList<RoiObject*>)),
            this, SLOT(objectsRemoved(QList<RoiObject*>)));

    //QPointF pos = mOriginIndicator->pos();
    scene->addItem(mOriginIndicator);

    for (ResizeHandle *handle : mResizeHandles) {
        //pos = handle->pos();
        scene->addItem(handle);
    }
}

void ObjectSelectionTool::deactivate(RoiScene *scene)
{
    scene->removeItem(mOriginIndicator);
    for (ResizeHandle *handle : mResizeHandles)
        scene->removeItem(handle);

    disconnect(mapDocument(), SIGNAL(objectsChanged(QList<RoiObject*>)),
               this, SLOT(updateHandles()));
    disconnect(mapDocument(), SIGNAL(mapChanged()),
               this, SLOT(updateHandles()));
    disconnect(scene, SIGNAL(selectedObjectItemsChanged()),
               this, SLOT(updateHandles()));

    AbstractObjectTool::deactivate(scene);
}

void ObjectSelectionTool::keyPressed(QKeyEvent *event)
{
    if (mAction != NoAction) {
        event->ignore();
        return;
    }

    QPointF moveBy;

    switch (event->key()) {
    case Qt::Key_Up:    moveBy = QPointF(0, -1); break;
    case Qt::Key_Down:  moveBy = QPointF(0, 1); break;
    case Qt::Key_Left:  moveBy = QPointF(-1, 0); break;
    case Qt::Key_Right: moveBy = QPointF(1, 0); break;
    default:
        AbstractObjectTool::keyPressed(event);
        return;
    }

    const QSet<RoiObjectItem*> &items = mapScene()->selectedObjectItems();
    const Qt::KeyboardModifiers modifiers = event->modifiers();

    if (moveBy.isNull() || items.isEmpty() || (modifiers & Qt::ControlModifier)) {
        event->ignore();
        return;
    }

    const bool moveFast = modifiers & Qt::ShiftModifier;
    //const bool snapToFineGrid = false;//Preferences::instance()->snapToFineGrid();

    if (moveFast) {
        // TODO: This only makes sense for orthogonal maps
        moveBy.rx() *= mapDocument()->roimap()->roiWidth();
        moveBy.ry() *= mapDocument()->roimap()->roiHeight();
//        if (snapToFineGrid)
//            moveBy /= Preferences::instance()->gridFine();
    }

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s)", "", items.size()));
    int i = 0;
    for (RoiObjectItem *objectItem : items) {
        RoiObject *object = objectItem->roiObject();
        const QPointF oldPos = object->position();
        const QPointF newPos = oldPos + moveBy;
        undoStack->push(new MoveRoiObject(mapDocument(), object, newPos, oldPos));
        ++i;
    }
    undoStack->endMacro();
}

void ObjectSelectionTool::mouseEntered()
{
}

void ObjectSelectionTool::mouseMoved(const QPointF &posIn,
                                     Qt::KeyboardModifiers modifiers)
{
    QPointF pos = posIn;

    RasterImageView *pView = mapDocument()->imageView();
    if (!pView)
        return;
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();
    pos.setX(pos.x() - offset.x());
    pos.setY(pos.y() - offset.y());
    //qDebug() << "mouseMoved" << posIn << scroll << offset << scale << pos;

    AbstractObjectTool::mouseMoved(pos, modifiers);

    updateHover(pos);

    if (mAction == NoAction && mMousePressed) {
        QPoint screenPos = QCursor::pos();
        const int dragDistance = (mScreenStart - screenPos).manhattanLength() * scale;

        // Use a reduced start drag distance to increase the responsiveness
        if (dragDistance >= QApplication::startDragDistance() / 2) {
            const bool hasSelection = !mapScene()->selectedObjectItems().isEmpty();

            // Holding Alt forces moving current selection
            // Holding Shift forces selection rectangle
            if (mClickedOriginIndicator) {
                startMovingOrigin(pos);
            } else if (mClickedResizeHandle) {
                startResizing(pos);
            } else if ((mClickedObjectItem || ((modifiers & Qt::AltModifier) && hasSelection)) &&
                       !(modifiers & Qt::ShiftModifier)) {
                startMoving(pos, modifiers);
            } else {
                startSelecting();
            }
        }
    }

    switch (mAction) {
    case Selecting:
        //qDebug() << "Seleting" << mStart-scroll << pos;
        mSelectionRectangle->setRectangle(QRectF(mStart*scale+scroll, pos*scale+scroll).normalized());
        //mSelectionRectangle->setOffset(QPointF(0,0));//offset-scroll);
        //mSelectionRectangle->setZoom(scale);
        break;
    case Moving:
        updateMovingItems(pos, modifiers);
        break;
    case MovingOrigin:
        updateMovingOrigin(pos, modifiers);
        break;
    case Resizing:
        updateResizingItems(pos, modifiers);
        break;
    case NoAction:
        break;
    }

    refreshCursor();
}

static QGraphicsView *findView(QGraphicsSceneEvent *event)
{
    if (QWidget *viewport = event->widget())
        return qobject_cast<QGraphicsView*>(viewport->parent());
    return nullptr;
}

void ObjectSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (mAction != NoAction) // Ignore additional presses during select/move
        return;

    QPointF pos = event->scenePos();
    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    //QPointF scroll = pView->scrollPos();
    //const qreal scale = mapDocument()->zoom();
    pos.setX(pos.x() - offset.x());
    pos.setY(pos.y() - offset.y());
    //qDebug() << "mousePressed" << event->scenePos() << scroll <<  offset << scale << pos;

    switch (event->button()) {
    case Qt::LeftButton: {
        mMousePressed = true;
        mStart = pos;
        mScreenStart = event->screenPos();

        Handle *clickedHandle = nullptr;

        if (QGraphicsView *view = findView(event))
        {
            //QGraphicsView *view = mapScene()->views().first();
            QGraphicsItem *clickedItem = mapScene()->itemAt(pos,
                                                            view->transform());

            clickedHandle = dynamic_cast<Handle*>(clickedItem);
        }

        if (!clickedHandle) {
            mClickedObjectItem = topMostObjectItemAt(pos);
        } else {
            mClickedOriginIndicator = dynamic_cast<OriginIndicator*>(clickedHandle);
            mClickedResizeHandle = dynamic_cast<ResizeHandle*>(clickedHandle);
        }
        mOldOriginPosition = mOriginIndicator->pos();

        break;
    }
    case Qt::RightButton:
        if (event->modifiers() & Qt::AltModifier) {
            QList<RoiObjectItem*> underlyingObjects = objectItemsAt(pos);
            if (underlyingObjects.empty())
                break;
            QMenu selectUnderlyingMenu;

            for (int levelNum = 0; levelNum < underlyingObjects.size(); ++levelNum) {
                QString objectName = underlyingObjects[levelNum]->roiObject()->name();
                const QString& objectType = underlyingObjects[levelNum]->roiObject()->type();
                if (objectName.isEmpty()) {
                    if (objectType.isEmpty())
                        objectName = tr("Unnamed object");
                    else
                        objectName = tr("Instance of %1").arg(objectType);
                }
                QString actionName;
                if (levelNum < 9)
                    actionName = tr("&%1) %2").arg(levelNum + 1).arg(objectName);
                else
                    actionName = tr("%1) %2").arg(levelNum + 1).arg(objectName);
                QAction *action = selectUnderlyingMenu.addAction(actionName);
                action->setData(QVariant::fromValue(underlyingObjects[levelNum]));
            }

            QAction *action = selectUnderlyingMenu.exec(event->screenPos());

            if (!action)
                break;

            if (RoiObjectItem* objectToBeSelected = action->data().value<RoiObjectItem*>()) {
                auto selection = mapScene()->selectedObjectItems();
                if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
                    if (selection.contains(objectToBeSelected))
                        selection.remove(objectToBeSelected);
                    else
                        selection.insert(objectToBeSelected);
                } else {
                    selection.clear();
                    selection.insert(objectToBeSelected);
                }
                mapScene()->setSelectedObjectItems(selection);
            }
        } else {
            AbstractObjectTool::mousePressed(event);
        }
        break;
    default:
        AbstractObjectTool::mousePressed(event);
        break;
    }
}

void ObjectSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QPointF pos = event->scenePos();
    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    //QPointF scroll = pView->scrollPos();
    //const qreal scale = mapDocument()->zoom();
    pos.setX(pos.x() - offset.x());
    pos.setY(pos.y() - offset.y());

    //qDebug() << "mouseReleased" << event->scenePos() << scroll <<  offset << scale << pos;

    switch (mAction) {
    case NoAction: {
        if (mClickedOriginIndicator || mClickedResizeHandle) {
            // Don't change selection as a result of clicking on a handle
            break;
        }
        const Qt::KeyboardModifiers modifiers = event->modifiers();
        QSet<RoiObjectItem*> selection = mapScene()->selectedObjectItems();
        if (modifiers & Qt::AltModifier) {
            const auto underlyingObjects = objectItemsAt(pos);
            if (underlyingObjects.isEmpty())
                break;

            // Determine the item after the last selected item
            RoiObjectItem *nextItem = underlyingObjects.first();
            for (int i = underlyingObjects.size() - 1; i >= 0; --i) {
                RoiObjectItem *underlyingObject = underlyingObjects.at(i);
                if (selection.contains(underlyingObject))
                    break;
                nextItem = underlyingObject;
            }

            // If the first and last item are already selected, try to find the
            // first non-selected item. If even that fails, we pretend to have
            // clicked the first item as usual to allow toggling the selection.
            if (selection.contains(nextItem)) {
                for (int i = 1; i < underlyingObjects.size() - 1; ++i) {
                    RoiObjectItem *underlyingObject = underlyingObjects.at(i);
                    if (!selection.contains(underlyingObject)) {
                        nextItem = underlyingObject;
                        break;
                    }
                }
            }

            mClickedObjectItem = nextItem;
        }
        if (mClickedObjectItem) {
            if (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) {
                if (selection.contains(mClickedObjectItem))
                    selection.remove(mClickedObjectItem);
                else
                    selection.insert(mClickedObjectItem);
                mapScene()->setSelectedObjectItems(selection);
            } else if (selection.contains(mClickedObjectItem)) {
                // Clicking one of the selected items changes the edit mode
                setMode((mMode == Resize) ? Rotate : Resize);
            } else {
                selection.clear();
                selection.insert(mClickedObjectItem);
                setMode(Resize);
                mapScene()->setSelectedObjectItems(selection);
            }
        } else if (!(modifiers & Qt::ShiftModifier)) {
            mapScene()->setSelectedObjectItems(QSet<RoiObjectItem*>());
        }
        break;
    }
    case Selecting:
        updateSelection(pos, event->modifiers());
        mapScene()->removeItem(mSelectionRectangle);
        mAction = NoAction;
        break;
    case Moving:
        finishMoving(pos);
        //mOldOriginPosition = mOriginIndicator->pos();
        break;
    case MovingOrigin:
        finishMovingOrigin();
        break;
    case Resizing:
        finishResizing(pos);
        break;
    }

    mMousePressed = false;
    mClickedObjectItem = nullptr;
    mClickedOriginIndicator = nullptr;
    mClickedResizeHandle = nullptr;

    updateHover(pos);
    refreshCursor();


//    updateHandles();
}

void ObjectSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;
    refreshCursor();
}

void ObjectSelectionTool::languageChanged()
{
    setName(tr("Select Objects"));
    setShortcut(QKeySequence(tr("S")));
}

static QPointF alignmentOffset(const QRectF &r, Alignment alignment)
{
    switch (alignment) {
    case TopLeft:       break;
    case Top:           return QPointF(r.width() / 2, 0);               break;
    case TopRight:      return QPointF(r.width(), 0);                   break;
    case Left:          return QPointF(0, r.height() / 2);              break;
    case Center:        return QPointF(r.width() / 2, r.height() / 2);  break;
    case Right:         return QPointF(r.width(), r.height() / 2);      break;
    case BottomLeft:    return QPointF(0, r.height());                  break;
    case Bottom:        return QPointF(r.width() / 2, r.height());      break;
    case BottomRight:   return QPointF(r.width(), r.height());          break;
    }
    return QPointF();
}

// TODO: Check whether this function should be moved into RoiObject::bounds
static void align(QRectF &r, Alignment alignment)
{
    r.translate(-alignmentOffset(r, alignment));
}

static void unalign(QRectF &r, Alignment alignment)
{
    r.translate(alignmentOffset(r, alignment));
}

static QRectF pixelBounds(const RoiObject *object)
{
    switch (object->shape()) {
    case RoiObject::Ellipse:
    case RoiObject::Pattern:
    case RoiObject::Rectangle:
    case RoiObject::Point: {
        QRectF bounds(object->bounds());
        align(bounds, object->alignment());
        return bounds;
    }
    case RoiObject::Polygon:
    case RoiObject::Polyline: {
        // Alignment is irrelevant for polygon objects since they have no size
        //const QPointF &pos = object->position();
        const QPolygonF polygon = object->polygon();//.translated(pos);
        return polygon.boundingRect();
    }
    case RoiObject::Text:
        Q_ASSERT(false);  // text objects only have screen bounds
        break;
    }

    return QRectF();
}

static bool resizeInPixelSpace(const RoiObject *object)
{
    return object->shape() != RoiObject::Text;
}

static bool canResizeAbsolute(const RoiObject *object)
{
    switch (object->shape()) {
    case RoiObject::Rectangle:
    case RoiObject::Ellipse:
    case RoiObject::Text:
    case RoiObject::Pattern:
    case RoiObject::Point:
        return true;
    case RoiObject::Polygon:
    case RoiObject::Polyline:
        return false;
    }

    return false;
}

/* This function returns the actual bounds of the object, as opposed to the
 * bounds of its visualization that the RoiRenderer::boundingRect function
 * returns.
 *
 * Before calculating the final bounding rectangle, the object is transformed
 * by the given transformation.
 */
static QRectF objectBounds(const RoiObject *object,
                           const RoiRenderer *renderer,
                           const QTransform &transform)
{
    {
        switch (object->shape()) {
        case RoiObject::Ellipse:
        case RoiObject::Pattern:
        case RoiObject::Rectangle:
        case RoiObject::Point: {
            QRectF bounds(object->bounds());

            align(bounds, object->alignment());
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(bounds);

            return transform.map(screenPolygon).boundingRect();
        }
        case RoiObject::Polygon:
        case RoiObject::Polyline: {
            // Alignment is irrelevant for polygon objects since they have no size
            //const QPointF &pos = object->position();
            const QPolygonF polygon = object->polygon();//.translated(pos);
            QPolygonF screenPolygon = renderer->pixelToScreenCoords(polygon);
            return transform.map(screenPolygon).boundingRect();
        }
        case RoiObject::Text: {
            const auto rect = renderer->boundingRect(object);
            return transform.mapRect(rect);
        }
        }
    }

    return QRectF();
}

static QTransform rotateAt(const QPointF &position, qreal rotation)
{
    QTransform transform;
    transform.translate(position.x(), position.y());
    transform.rotate(rotation);
    transform.translate(-position.x(), -position.y());
    return transform;
}

static QTransform objectTransform(RoiObject *object, RoiRenderer *renderer)
{
    if (object->objectGroup() == nullptr)
        return QTransform();
    QTransform transform;

    if (object->rotation() != 0) {
        const QPointF pos = renderer->pixelToScreenCoords(object->position());
        transform = rotateAt(pos, object->rotation());
    }

    QPointF offset = object->objectGroup()->totalOffset();
    if (!offset.isNull())
        transform *= QTransform::fromTranslate(offset.x(), offset.y());

    return transform;
}

void ObjectSelectionTool::updateHandles(bool resetOriginIndicator)
{
    if (mAction == Moving || mAction == Resizing)
        return;

    RasterImageView *pView = mapDocument()->imageView();
    if (pView == nullptr)
        return;
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();
    const qreal scale = mapDocument()->zoom();

    const QList<RoiObject*> &objects = mapDocument()->selectedObjects();
    const bool showHandles = objects.size() > 0;

    if (showHandles) {
        RoiRenderer *renderer = mapDocument()->renderer();
        QRectF boundingRect = objectBounds(objects.first(), renderer,
                                           objectTransform(objects.first(), renderer));

        for (int i = 1; i < objects.size(); ++i) {
            RoiObject *object = objects.at(i);
            boundingRect |= objectBounds(object, renderer,
                                         objectTransform(object, renderer));
        }

        QPointF topLeft = boundingRect.topLeft();
        QPointF topRight = boundingRect.topRight();
        QPointF bottomLeft = boundingRect.bottomLeft();
        QPointF bottomRight = boundingRect.bottomRight();
        QPointF center = boundingRect.center();

        qreal handleRotation = 0;

        // If there is only one object selected, align to its orientation.
        if (objects.size() == 1) {
            RoiObject *object = objects.first();
            //if (!object->objectGroup()->isVisible() || !object->isVisible())
            //    return;

            handleRotation = object->rotation();

            if (resizeInPixelSpace(object)) {
                QRectF bounds = pixelBounds(object);

                QTransform transform(objectTransform(object, renderer));
                topLeft = transform.map(renderer->pixelToScreenCoords(bounds.topLeft()));
                topRight = transform.map(renderer->pixelToScreenCoords(bounds.topRight()));
                bottomLeft = transform.map(renderer->pixelToScreenCoords(bounds.bottomLeft()));
                bottomRight = transform.map(renderer->pixelToScreenCoords(bounds.bottomRight()));
                center = transform.map(renderer->pixelToScreenCoords(bounds.center()));

                // Ugly hack to make handles appear nicer in this case
                if (mapDocument()->roimap()->orientation() == RoiMap::Isometric)
                    handleRotation += 45;

            } else {
                QRectF bounds = objectBounds(object, renderer, QTransform());

                QTransform transform(objectTransform(object, renderer));
                topLeft = transform.map(bounds.topLeft());
                topRight = transform.map(bounds.topRight());
                bottomLeft = transform.map(bounds.bottomLeft());
                bottomRight = transform.map(bounds.bottomRight());
                center = transform.map(bounds.center());
            }
        }

        center = center - scroll + offset;
        if (resetOriginIndicator) {
            mOriginIndicator->setMapDocument(mapDocument());
            mOriginIndicator->setPos(center);
        }

        topLeft = topLeft - scroll + offset/scale;
        topRight = topRight - scroll + offset/scale;
        bottomLeft = bottomLeft - scroll + offset/scale;
        bottomRight = bottomRight - scroll + offset/scale;

        QPointF top = (topLeft + topRight) / 2;
        QPointF left = (topLeft + bottomLeft) / 2;
        QPointF right = (topRight + bottomRight) / 2;
        QPointF bottom = (bottomLeft + bottomRight) / 2;


        mResizeHandles[TopAnchor]->setPos(top);
        mResizeHandles[TopAnchor]->setResizingOrigin(bottom);
        mResizeHandles[LeftAnchor]->setPos(left);
        mResizeHandles[LeftAnchor]->setResizingOrigin(right);
        mResizeHandles[RightAnchor]->setPos(right);
        mResizeHandles[RightAnchor]->setResizingOrigin(left);
        mResizeHandles[BottomAnchor]->setPos(bottom);
        mResizeHandles[BottomAnchor]->setResizingOrigin(top);
        
        //qDebug() << "topLeft: " << topLeft;
        mResizeHandles[TopLeftAnchor]->setPos(topLeft);
        mResizeHandles[TopLeftAnchor]->setResizingOrigin(bottomRight);
        mResizeHandles[TopRightAnchor]->setPos(topRight);
        mResizeHandles[TopRightAnchor]->setResizingOrigin(bottomLeft);
        mResizeHandles[BottomLeftAnchor]->setPos(bottomLeft);
        mResizeHandles[BottomLeftAnchor]->setResizingOrigin(topRight);
        mResizeHandles[BottomRightAnchor]->setPos(bottomRight);
        mResizeHandles[BottomRightAnchor]->setResizingOrigin(topLeft);

        for (ResizeHandle *handle : mResizeHandles) {
            handle->setMapDocument(mapDocument());
            handle->setRotation(handleRotation);
        }

    }

    updateHandleVisibility();
}

void ObjectSelectionTool::updateHandleVisibility()
{
    const bool hasSelection = !mapDocument()->selectedObjects().isEmpty();
    const bool showHandles = hasSelection && (mAction == NoAction || mAction == Selecting);
    const bool showOrigin = hasSelection &&
            mAction != Moving && (mMode == Rotate || mAction == Resizing);

    for (ResizeHandle *handle : mResizeHandles)
        handle->setVisible(showHandles && mMode == Resize);

    mOriginIndicator->setVisible(showOrigin);
}

void ObjectSelectionTool::objectsRemoved(const QList<RoiObject *> &objects)
{
    if (mAction != Moving && mAction != Resizing)
        return;

    // Abort move/rotate/resize to avoid crashing...
    // TODO: This should really not be allowed to happen in the first place
    // since it breaks the undo history, for example.
    for (int i = mMovingObjects.size() - 1; i >= 0; --i) {
        const MovingObject &object = mMovingObjects.at(i);
        RoiObject *roiObject = object.item->roiObject();

        if (objects.contains(roiObject)) {
            // Avoid referencing the removed object
            mMovingObjects.remove(i);
        } else {
            roiObject->setPosition(object.oldPosition);
            roiObject->setSize(object.oldSize);
            roiObject->setPolygon(object.oldPolygon);
            roiObject->setRotation(object.oldRotation);
        }
    }

    emit mapDocument()->roiObjectModel()->objectsChanged(changingObjects());

    mMovingObjects.clear();
}

void ObjectSelectionTool::updateHover(const QPointF &pos)
{
    Handle *hoveredHandle = nullptr;

    if (mClickedOriginIndicator) {
        hoveredHandle = mClickedOriginIndicator;
    } else if (mClickedResizeHandle) {
        hoveredHandle = mClickedResizeHandle;
    } else if (QGraphicsView *view = mapScene()->views().first()) {
        QGraphicsItem *hoveredItem = mapScene()->itemAt(pos,
                                                        view->transform());

        hoveredHandle = dynamic_cast<Handle*>(hoveredItem);
        if (hoveredHandle != nullptr) {
            //qDebug() << "hoveredHandle1: " << hoveredHandle;
        }
    }

    if (mHoveredHandle != hoveredHandle) {
        if (mHoveredHandle)
            mHoveredHandle->setUnderMouse(false);
        if (hoveredHandle)
            hoveredHandle->setUnderMouse(true);
        mHoveredHandle = hoveredHandle;
    }

    RoiObjectItem *hoveredObjectItem = nullptr;
    if (!hoveredHandle) {
        hoveredObjectItem = topMostObjectItemAt(pos);
        if (hoveredObjectItem != nullptr) {
            //qDebug() << "hoveredObjectItem2: " << hoveredObjectItem;
        }
    }
    mHoveredObjectItem = hoveredObjectItem;
}

void ObjectSelectionTool::updateSelection(const QPointF &pos,
                                          Qt::KeyboardModifiers modifiers)
{
    QRectF rect = QRectF(mStart, pos).normalized();

    // Make sure the rect has some contents, otherwise intersects returns false
    rect.setWidth(qMax(qreal(1), rect.width()));
    rect.setHeight(qMax(qreal(1), rect.height()));

    QSet<RoiObjectItem*> selectedItems;

    const QList<QGraphicsItem *> &items = mapScene()->items(rect);
    for (QGraphicsItem *item : items) {
        RoiObjectItem *roiObjectItem = dynamic_cast<RoiObjectItem*>(item);
        if (roiObjectItem)
            selectedItems.insert(roiObjectItem);
    }

    if (modifiers & (Qt::ControlModifier | Qt::ShiftModifier))
        selectedItems |= mapScene()->selectedObjectItems();
    else
        setMode(Resize);

    mapScene()->setSelectedObjectItems(selectedItems);
}

void ObjectSelectionTool::startSelecting()
{
    mAction = Selecting;
    mapScene()->addItem(mSelectionRectangle);
}

void ObjectSelectionTool::startMoving(const QPointF &pos,
                                      Qt::KeyboardModifiers modifiers)
{
    // Move only the clicked item, if it was not part of the selection
    if (mClickedObjectItem && !(modifiers & Qt::AltModifier)) {
        if (!mapScene()->selectedObjectItems().contains(mClickedObjectItem))
            mapScene()->setSelectedObjectItems(QSet<RoiObjectItem*>() << mClickedObjectItem);
    }

    //qDebug() << "startMoving" << pos;
    saveSelectionState();

    mStart = pos;
    mAction = Moving;
    mAlignPosition = mMovingObjects.first().oldPosition;
    mOldOriginPosition = mOriginIndicator->pos();

    foreach (const MovingObject &object, mMovingObjects) {
        const QPointF &pos = object.oldPosition;
        if (pos.x() < mAlignPosition.x())
            mAlignPosition.setX(pos.x());
        if (pos.y() < mAlignPosition.y())
            mAlignPosition.setY(pos.y());
    }

    updateHandleVisibility();
}

void ObjectSelectionTool::updateMovingItems(const QPointF &posIn,
                                            Qt::KeyboardModifiers modifiers)
{
    const RoiRenderer *renderer = mapDocument()->renderer();
    QPointF diff = snapToGrid(posIn - mStart, modifiers);

    const qreal scale = mapDocument()->zoom();
    diff = QPointF(diff.x(), diff.y()) / scale;

    foreach (const MovingObject &object, mMovingObjects) {
        const QPointF newPixelPos = object.oldPosition + diff;
        const QPointF newPos = renderer->screenToPixelCoords(newPixelPos);

        RoiObject *roiObject = object.item->roiObject();
        if (newPos.x()+roiObject->width()/2 < 0 || newPos.y()+roiObject->height()/2 < 0)
            return;

        if (roiObject->mParent != nullptr)
        {
            if (newPos.x() < roiObject->mParent->x() || newPos.y() < roiObject->mParent->y())
                return;
            if (newPos.x()+roiObject->width() > roiObject->mParent->x()+roiObject->mParent->width() ||
                    newPos.y()+roiObject->height() > roiObject->mParent->y()+roiObject->mParent->height())
                return;
        }

        RasterImageView *pView = mapDocument()->imageView();
        const QImage *pimg = &pView->document()->image();
        if (newPos.x()+roiObject->width()/2 > pimg->width() || newPos.y()+roiObject->height()/2 > pimg->height())
            return;

        roiObject->setOldPosition(roiObject->position());
        roiObject->setPosition(newPos);

        //qDebug() << newPos;
    }

    mapDocument()->roiObjectModel()->emitObjectsChanged(changingObjects(), RoiObjectModel::Position);

    mOriginIndicator->setPos(mOldOriginPosition + diff);
}

void ObjectSelectionTool::finishMoving(const QPointF &pos)
{
    Q_ASSERT(mAction == Moving);
    mAction = NoAction;
    updateHandles(false);

    if (mStart == pos) // Move is a no-op
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s)", "", mMovingObjects.size()));
    foreach (const MovingObject &object, mMovingObjects) {
        undoStack->push(new MoveRoiObject(mapDocument(),
                                          object.item->roiObject(),
                                          object.oldPosition));
    }
    undoStack->endMacro();

    mMovingObjects.clear();
}

void ObjectSelectionTool::startMovingOrigin(const QPointF &pos)
{
    mStart = pos;
    mAction = MovingOrigin;
    mOldOriginPosition = mOriginIndicator->pos();
}

void ObjectSelectionTool::updateMovingOrigin(const QPointF &pos,
                                             Qt::KeyboardModifiers)
{
    mOriginIndicator->setPos(mOldOriginPosition + (pos - mStart));
}

void ObjectSelectionTool::finishMovingOrigin()
{
    Q_ASSERT(mAction == MovingOrigin);
    mAction = NoAction;
    mOldOriginPosition = mOriginIndicator->pos();
}

void ObjectSelectionTool::startResizing(const QPointF &pos)
{
    mStart = pos;
    //qDebug() << "startResizing" << mStart;
    mAction = Resizing;
    mOrigin = mStart - mOriginIndicator->pos();

    mResizingLimitHorizontal = mClickedResizeHandle->resizingLimitHorizontal();
    mResizingLimitVertical = mClickedResizeHandle->resizingLimitVertical();

    saveSelectionState();
    updateHandleVisibility();
}

void ObjectSelectionTool::updateResizingItems(const QPointF &pos,
                                              Qt::KeyboardModifiers modifiers)
{
    RoiRenderer *renderer = mapDocument()->renderer();

    QPointF resizingOrigin = mClickedResizeHandle->resizingOrigin();
    if (modifiers & Qt::ShiftModifier)
        resizingOrigin = mOrigin;

    mOriginIndicator->setPos(resizingOrigin);

    QPointF pixelPos = renderer->screenToPixelCoords(pos);
    QPointF snappedScreenPos = renderer->pixelToScreenCoords(pixelPos);

    if (mMovingObjects.size() == 1) {
        /* For single items the resizing is performed in object space in order
         * to handle different scaling on X and Y axis as well as to improve
         * handling of 0-sized objects.
         */
        updateResizingSingleItem(resizingOrigin, snappedScreenPos, modifiers);
        return;
    }

    QPointF diff = snappedScreenPos - resizingOrigin;
    QPointF startDiff = mStart - resizingOrigin;

    /* Calculate the scaling factor. Minimum is 1% to protect against making
     * everything 0-sized and non-recoverable (it's still possibly to run into
     * problems by repeatedly scaling down to 1%, but that's asking for it)
     */
    qreal scale;
    if (mResizingLimitHorizontal) {
        scale = qMax((qreal)0.01, diff.y() / startDiff.y());
    } else if (mResizingLimitVertical) {
        scale = qMax((qreal)0.01, diff.x() / startDiff.x());
    } else {
        scale = qMin(qMax((qreal)0.01, diff.x() / startDiff.x()),
                     qMax((qreal)0.01, diff.y() / startDiff.y()));
    }

    if (!std::isfinite(scale))
        scale = 1;

    foreach (const MovingObject &object, mMovingObjects) {
        RoiObject *roiObject = object.item->roiObject();
        const QPointF offset = roiObject->objectGroup()->totalOffset();

        const QPointF oldRelPos = object.oldPosition + offset - resizingOrigin;
        const QPointF scaledRelPos(oldRelPos.x() * scale,
                                   oldRelPos.y() * scale);
        const QPointF newScreenPos = resizingOrigin + scaledRelPos - offset;
        const QPointF newPos = renderer->screenToPixelCoords(newScreenPos);
        const QSizeF origSize = object.oldSize;
        const QSizeF newSize(origSize.width() * scale,
                             origSize.height() * scale);

        if (roiObject->polygon().isEmpty() == false) {
            // For polygons, we have to scale in object space.
            qreal rotation = object.item->rotation() * M_PI / -180;
            const qreal sn = std::sin(rotation);
            const qreal cs = std::cos(rotation);
            
            const QPolygonF &oldPolygon = object.oldPolygon;
            QPolygonF newPolygon(oldPolygon.size());
            for (int n = 0; n < oldPolygon.size(); ++n) {
                const QPointF oldPoint(oldPolygon[n]);
                const QPointF rotPoint(oldPoint.x() * cs + oldPoint.y() * sn,
                                       oldPoint.y() * cs - oldPoint.x() * sn);
                const QPointF scaledPoint(rotPoint.x() * scale, rotPoint.y() * scale);
                const QPointF newPoint(scaledPoint.x() * cs - scaledPoint.y() * sn,
                                       scaledPoint.y() * cs + scaledPoint.x() * sn);
                newPolygon[n] = newPoint;
            }
            roiObject->setPolygon(newPolygon);
        }
        
        roiObject->setSize(newSize);
        roiObject->setPosition(newPos);
    }

    mapDocument()->roiObjectModel()->emitObjectsChanged(changingObjects(), RoiObjectModel::Position);
}

void ObjectSelectionTool::updateResizingSingleItem(const QPointF &resizingOrigin,
                                                   const QPointF &screenPos,
                                                   Qt::KeyboardModifiers modifiers)
{
    const RoiRenderer *renderer = mapDocument()->renderer();
    const MovingObject &object = mMovingObjects.first();
    RoiObject *roiObject = object.item->roiObject();
    const qreal scale = mapDocument()->zoom();

    /* The resizingOrigin, screenPos and mStart are affected by the ObjectGroup
     * goffset. We will un-apply it to these variables since the resize for
     * single items happens in local coordinate space.
     */
    QPointF goffset = roiObject->objectGroup()->totalOffset();

    QPointF origin = (resizingOrigin - goffset);
    QPointF pos = (screenPos - goffset);
    QPointF oldPos = object.oldPosition;

    origin = QPointF(origin.x(), origin.y());

    RasterImageView *pView = mapDocument()->imageView();
    QPointF offset = pView->imageOffset();
    QPointF scroll = pView->scrollPos();

    //qDebug() << "pos" << pos << "mStart" << mStart << "screenPos" << screenPos << "goffset" << goffset;
    QPointF diff = snapToGrid(pos - mStart, modifiers);
    diff = QPointF(diff.x(), diff.y()) / scale;

    pos.setX((pos.x() - offset.x()) + scroll.x());
    pos.setY((pos.y() - offset.y()) + scroll.y());

    QPointF newPos = oldPos;
    QSizeF newSize = object.oldSize;

    /* In case one of the anchors was used as-is, the desired size can be
     * derived directly from the distance from the origin for rectangle
     * and ellipse objects. This allows scaling up a 0-sized object without
     * dealing with infinite scaling factor issues.
     *
     * For obvious reasons this can't work on polygons or polylines, nor when
     * preserving the aspect ratio.
     */
    if (mClickedResizeHandle->resizingOrigin() == resizingOrigin &&
            canResizeAbsolute(roiObject)) {

        QRectF newBounds = QRectF(newPos, newSize);
        align(newBounds, roiObject->alignment());

        //qDebug() << "newBounds" << newBounds << "diff" << diff;
        //diff = QPointF();

        //QPointF p1 = pos;
        //p1 = pos / scale + scroll / scale;

        switch (mClickedResizeHandle->anchorPosition()) {
        case LeftAnchor:
        case TopLeftAnchor:
        case BottomLeftAnchor:
            newBounds.setLeft(newBounds.left() + diff.x());
            break;
        case RightAnchor:
        case TopRightAnchor:
        case BottomRightAnchor:
            newBounds.setRight(newBounds.right() + diff.x());
            break;
        default:
            // nothing to do on this axis
            break;
        }

        switch (mClickedResizeHandle->anchorPosition()) {
        case TopAnchor:
        case TopLeftAnchor:
        case TopRightAnchor:
            newBounds.setTop(newBounds.top() + diff.y());
            break;
        case BottomAnchor:
        case BottomLeftAnchor:
        case BottomRightAnchor:
            newBounds.setBottom(newBounds.bottom() + diff.y());
            break;
        default:
            // nothing to do on this axis
            break;
        }

        unalign(newBounds, roiObject->alignment());

        newSize = newBounds.size();
        newPos = newBounds.topLeft();
    } else {
        QPointF oldRelPos = oldPos - origin;
        newPos = origin + QPointF(oldRelPos.x() * scale,
                                  oldRelPos.y() * scale);

        if (!object.oldPolygon.isEmpty()) {
            QPolygonF newPolygon(object.oldPolygon.size());
            for (int n = 0; n < object.oldPolygon.size(); ++n) {
                const QPointF &point = object.oldPolygon[n];
                newPolygon[n] = QPointF(point.x() * scale,
                                        point.y() * scale);
            }
            roiObject->setPolygon(newPolygon);
        }
    }

    newPos = renderer->screenToPixelCoords(newPos);

    roiObject->setSize(newSize);
    roiObject->setPosition(newPos);

    mapDocument()->roiObjectModel()->emitObjectsChanged(changingObjects(), RoiObjectModel::Position);
}

void ObjectSelectionTool::finishResizing(const QPointF &pos)
{
    Q_ASSERT(mAction == Resizing);
    mAction = NoAction;
    updateHandles();

    if (mStart == pos) // No scaling at all
        return;

    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Resize %n Object(s)", "", mMovingObjects.size()));
    foreach (const MovingObject &object, mMovingObjects) {
        RoiObject *roiObject = object.item->roiObject();
        undoStack->push(new MoveRoiObject(mapDocument(), roiObject, object.oldPosition));
        undoStack->push(new ResizeRoiObject(mapDocument(), roiObject, object.oldSize));
        
        if (!object.oldPolygon.isEmpty())
            undoStack->push(new ChangePolygon(mapDocument(), roiObject, object.oldPolygon));
    }
    undoStack->endMacro();

    mMovingObjects.clear();

    //qDebug() << "finishResizing " << pos;
}

void ObjectSelectionTool::setMode(Mode mode)
{
    if (mMode != mode) {
        mMode = mode;
        updateHandles(false);
    }
}

void ObjectSelectionTool::saveSelectionState()
{
    mMovingObjects.clear();

    // Remember the initial state before moving, resizing or rotating
    for (RoiObjectItem *item : mapScene()->selectedObjectItems()) {
        RoiObject *roiObject = item->roiObject();
        MovingObject object = {
            item,
            //item->pos(),
            roiObject->position(),
            roiObject->size(),
            roiObject->polygon(),
        };
        mMovingObjects.append(object);
    }
}

void ObjectSelectionTool::refreshCursor()
{
    Qt::CursorShape cursorShape = Qt::ArrowCursor;

    switch (mAction) {
    case NoAction: {
        const bool hasSelection = !mapScene()->selectedObjectItems().isEmpty();

        if ((mHoveredObjectItem || ((mModifiers & Qt::AltModifier) && hasSelection && !mHoveredHandle)) &&
                !(mModifiers & Qt::ShiftModifier)) {
            cursorShape = Qt::SizeAllCursor;
        }

        break;
    }
    case Moving:
        cursorShape = Qt::SizeAllCursor;
        break;
    default:
        break;
    }

    if (cursor().shape() != cursorShape)
        setCursor(cursorShape);
}

QPointF ObjectSelectionTool::snapToGrid(const QPointF &diff,
                                        Qt::KeyboardModifiers modifiers)
{
    RoiRenderer *renderer = mapDocument()->renderer();
    SnapHelper snapHelper(renderer, modifiers);

    if (snapHelper.snaps()) {
        const QPointF alignScreenPos = renderer->pixelToScreenCoords(mAlignPosition);
        const QPointF newAlignScreenPos = alignScreenPos + diff;

        QPointF newAlignPixelPos = renderer->screenToPixelCoords(newAlignScreenPos);
        snapHelper.snap(newAlignPixelPos);

        return renderer->pixelToScreenCoords(newAlignPixelPos) - alignScreenPos;
    }

    return diff;
}

QList<RoiObject *> ObjectSelectionTool::changingObjects() const
{
    QList<RoiObject*> changingObjects;
    changingObjects.reserve(mMovingObjects.size());

    foreach (const MovingObject &movingObject, mMovingObjects)
        changingObjects.append(movingObject.item->roiObject());

    return changingObjects;
}
