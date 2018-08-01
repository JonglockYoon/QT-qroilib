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
#ifndef RASTERIMAGEVIEW_H
#define RASTERIMAGEVIEW_H

#include <roilib_export.h>
#include <QTimer>

// Local
#include <qroilib/documentview/abstractimageview.h>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


class QGraphicsSceneHoverEvent;

namespace Qroilib
{

struct RasterImageViewPrivate;
class ROIDSHARED_EXPORT RasterImageView : public AbstractImageView
{
    Q_OBJECT
public:
    enum AlphaBackgroundMode {
        AlphaBackgroundCheckBoard,
        AlphaBackgroundSolid
    };

    RasterImageView(QGraphicsItem* parent = 0, int seq = 0);
    ~RasterImageView();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;

    void setAlphaBackgroundMode(AlphaBackgroundMode mode);
    void setAlphaBackgroundColor(const QColor& color);
    void setScalerSmooth(bool b);

protected:
    void loadFromDocument() Q_DECL_OVERRIDE;
    void onZoomChanged() Q_DECL_OVERRIDE;
    void onImageOffsetChanged() Q_DECL_OVERRIDE;
    void onScrollPosChanged(const QPointF& oldPos) Q_DECL_OVERRIDE;
    void resizeEvent(QGraphicsSceneResizeEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
    void wheelEvent(QGraphicsSceneWheelEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    void hoverMoveEvent(QGraphicsSceneHoverEvent*) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void updateBuffer(const QRegion& region = QRegion());

signals:
    void finishLoadDocument();

private Q_SLOTS:
    void finishSetDocument();
    void updateFromScaler(int, int, const QImage&);
    void updateImageRect(const QRect& imageRect);

private:
    RasterImageViewPrivate* const d;

    Qt::KeyboardModifiers mCurrentModifiers;
    QPointF mLastMousePos;
    bool mUnderMouse;
    int mSeq;
    bool bScalerSmooth;

};

} // namespace

#endif /* RASTERIMAGEVIEW_H */
