// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <agateau@kde.org>
Modify 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

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
#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <roilib_export.h>

// Qt
#include <QGraphicsWidget>
#include <QGraphicsScene>

#include <QDialogButtonBox>
#include <QDialog>
#include <QPushButton>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QDesktopWidget>
#include <QComboBox>

// Local
#include <qroilib/document/document.h>

#include "objectselectiontool.h"
#include "qtpropertymanager.h"
#include "qtvariantproperty.h"
#include "qttreepropertybrowser.h"

#include "roiwriter.h"
#include "roiobject.h"


class QPropertyAnimation;
class QUrl;

namespace Qroilib
{

class AbstractTool;
class RasterImageView;
class Layer;
class RoiScene;
class ToolManager;
class CreateObjectTool;
class BirdEyeView;

struct DocumentViewPrivate;

/**
 * This widget can display various documents, using an instance of
 * AbstractDocumentViewAdapter
 */
class ROIDSHARED_EXPORT DocumentView : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(bool zoomToFit READ zoomToFit WRITE setZoomToFit NOTIFY zoomToFitChanged)
    Q_PROPERTY(QPoint position READ position WRITE setPosition NOTIFY positionChanged)
public:
    QAction *actMoveTool;
    QAction *actSelectTool;
    QAction *actCreatePoint;
    QAction *actCreateRectangle;
    QAction *actCreatePattern;
    //QAction *act;

    ObjectSelectionTool *objSelTool;
    CreateObjectTool *pointObjectsTool;
    CreateObjectTool *rectangleObjectsTool;
    CreateObjectTool *patternroiObjectsTool;

    void ToolsSelect();
public:

    static const int MaximumZoom;
    static const int AnimDuration;

    RoiScene* mRoiScene;

    struct Setup {
        Setup()
        : valid(false)
        , zoomToFit(false)
        , zoom(0)
        {}
        bool valid:1;
        bool zoomToFit:1;
        qreal zoom;
        QPointF position;
    };

    /**
     * Create a new view attached to scene. We need the scene to be able to
     * install scene event filters.
     */
    DocumentView(RoiScene* scene, const QUrl url);
    ~DocumentView();

    Document::Ptr document() const;

    QUrl url() const;

    void openUrl(const QUrl&, const Setup&, int seq);

    Setup setup();
    QUndoStack *undoStack() const;

    /**
     * Tells the current adapter to load its config. Used when the user changed
     * the config while the view was visible.
     */
    void loadAdapterConfig();

    bool canZoom() const;

    qreal minimumZoom() const;

    qreal zoom() const;

    bool isCurrent() const;

    void setCurrent(bool);

    void setCompareMode(bool);

    bool zoomToFit() const;

    QPoint position();

    /**
     * Returns the RasterImageView of the current adapter, if it has one
     */
    RasterImageView* imageView() const;

    void moveTo(const QRect&);

    void setGeometry(const QRectF& rect) Q_DECL_OVERRIDE;

    int sortKey() const;

    /**
     * If true, areas around the document will be painted with the default brush.
     * If false they will be kept transparent.
     */
    void setEraseBorders(bool);

    QString ChannelName(const QUrl url);
    void InitRoi(const QUrl url);

    const QImage* image();
    const ParamTable* getParamTable() {
        return (const ParamTable*)pParamTable;
    }
    void setParamTable(ParamTable* p) {
        pParamTable = p;
    }

public Q_SLOTS:
    void setZoom(qreal);
    void setZoomToFit(bool);
    void setPosition(const QPoint&);
    void hideAndDeleteLater();
    void finishLoadDocument();

Q_SIGNALS:
    /**
     * Emitted when the part has finished loading
     */
    void completed(int seq);
    void minimumZoomChanged(qreal);
    void zoomChanged(qreal);
    void adapterChanged();
    void focused(DocumentView*);
    void zoomToFitChanged(bool);
    void positionChanged();
    void fadeInFinished(DocumentView*);
    void contextMenuRequested();
    void updateLayout();
    void finishNewRoiObject();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0) Q_DECL_OVERRIDE;
    void wheelEvent(QGraphicsSceneWheelEvent* event) Q_DECL_OVERRIDE;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) Q_DECL_OVERRIDE;
    bool sceneEventFilter(QGraphicsItem*, QEvent*) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void finishOpenUrl(int seq);
    void slotCompleted();

    void zoomIn(const QPointF& center = QPointF(-1, -1));
    void zoomOut(const QPointF& center = QPointF(-1, -1));

    void slotZoomChanged(qreal);

    void emitFocused();

public Q_SLOTS:
    void zoomActualSize();

private:
    friend struct DocumentViewPrivate;
    DocumentViewPrivate* const d;
    void createAdapterForDocument(int seq);

protected:
    Object *mCurrentObject;             /**< Current properties object. */
    Layer* mCurrentLayer;
    ParamTable *pParamTable;

public:
    ObjectGroup *objGroup;
    void setSelectedTool(AbstractTool *tool);

public:
    virtual QList<Object*> currentObjects() const;
    void setCurrentObject(Object *object);
    Layer *currentLayer() const { return mCurrentLayer; }
    void setCurrentLayer(Layer *layer);
    void setProperty(Object *object, const QString &name, const QVariant &value);
    void setProperties(Object *object, const Properties &properties);
    void removeProperty(Object *object, const QString &name);
    RoiObjectModel *roiObjectModel() const { return mRoiObjectModel; }

    /**
     * Returns the roimap instance. Be aware that directly modifying the roimap will
     * not allow the GUI to update itself appropriately.
     */
    RoiMap *roimap() const { return mRoi; }

    void duplicateObjects(const QList<RoiObject*> &objects);
    void removeObjects(const QList<RoiObject*> &objects);
    void moveObjectsUp(const QList<RoiObject*> &objects);
    void moveObjectsDown(const QList<RoiObject*> &objects);

    /**
     * Returns the selected area of tiles.
     */
    const QRegion &selectedArea() const { return mSelectedArea; }

    /**
     * Sets the selected area of tiles.
     */
    void setSelectedArea(const QRegion &selection);


    /**
     * Returns the roimap renderer.
     */
    RoiRenderer *renderer() const { return mRenderer; }

    /**
     * Creates the roimap renderer. Should be called after changing the roimap
     * orientation.
     */
    void createRenderer();
    /**
     * Returns the list of selected objects.
     */
    const QList<RoiObject*> &selectedObjects() const
    { return mSelectedObjects; }

    /**
     * Sets the list of selected objects, emitting the selectedObjectsChanged
     * signal.
     */
    void setSelectedObjects(const QList<RoiObject*> &selectedObjects);

    Object *currentObject() const { return mCurrentObject; }

    BirdEyeView* mBirdEyeView;

    QAction *registerTool(AbstractTool *tool);
    void selectTool(QAction* act);
    void clearSelectedObjectItems();
    void addLayer(Layer *layer);

    bool bMultiView = false;
    int seq;

signals:
    /**
     * Emitted when the selected tile region changes. Sends the currently
     * selected region and the previously selected region.
     */
    void selectedAreaChanged(const QRegion &newSelection,
                             const QRegion &oldSelection);

    /**
     * Emitted when the list of selected objects changes.
     */
    void selectedObjectsChanged();

    /**
     * Emitted when the roimap size or its tile size changes.
     */
    void mapChanged();

    void layerAdded(Layer *layer);
    void layerAboutToBeRemoved(GroupLayer *parentLayer, int index);
    void layerRemoved(Layer *layer);
    void layerChanged(Layer *layer);

    /**
     * Emitted when the current layer changes.
     */
    void currentLayerChanged(Layer *layer);

    void objectsAdded(const QList<RoiObject*> &objects);
    void objectsInserted(ObjectGroup *objectGroup, int first, int last);
    void objectsRemoved(const QList<RoiObject*> &objects);
    void objectsChanged(const QList<RoiObject*> &objects);
    void objectsTypeChanged(const QList<RoiObject*> &objects);
    void objectsIndexChanged(ObjectGroup *objectGroup, int first, int last);

    void modifiedChanged();

    /**
     * Should be emitted when changing the color or drawing order of an object
     * group.
     */
    void objectGroupChanged(ObjectGroup *objectGroup);

    /**
     * Makes the Properties window visible and take focus.
     */
    void editCurrentObject();

    void propertyAdded(Object *object, const QString &name);
    void propertyRemoved(Object *object, const QString &name);
    void propertyChanged(Object *object, const QString &name);
    void propertiesChanged(Object *object);

signals:
    void scaleChanged(qreal scale);

private slots:
    void onObjectsRemoved(const QList<RoiObject*> &objects);

    void onRoiObjectModelRowsInserted(const QModelIndex &parent, int first, int last);
    void onRoiObjectModelRowsInsertedOrRemoved(const QModelIndex &parent, int first, int last);
    void onObjectsMoved(const QModelIndex &parent, int start, int end,
                        const QModelIndex &destination, int row);

    void onLayerAdded(Layer *layer);
    void onLayerAboutToBeRemoved(GroupLayer *groupLayer, int index);
    void onLayerRemoved(Layer *layer);

    void cursorChanged(const QCursor &cursor);

public:
    void deselectObjects(const QList<RoiObject*> &objects);
    void moveObjectIndex(const RoiObject *object, int count);

    RoiMap *mRoi;
    QRegion mSelectedArea;
    QList<RoiObject*> mSelectedObjects;
    RoiRenderer *mRenderer;
    RoiObjectModel *mRoiObjectModel;

    QUndoStack *mUndoStack;

public slots:
    void delete_(); // 'delete' is a reserved word
    void selectNone();

public:
    ToolManager *mToolManager;
    AbstractTool *mSelectedTool;

};

inline QUndoStack *DocumentView::undoStack() const
{
    return mUndoStack;
}

} // namespace

#endif /* DOCUMENTVIEW_H */
