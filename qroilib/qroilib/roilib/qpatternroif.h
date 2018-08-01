#ifndef QPATTERNROI_H
#define QPATTERNROI_H

#include <QRectF>

class QPatternRoiF
{
public:
    QPatternRoiF();
    QPatternRoiF(const QRectF &r);
    ~QPatternRoiF();

    qreal x() const ;
    qreal y() const ;
    qreal width() const ;
    qreal height() const ;

    void setRect(const QRectF &r);
    QRectF getRect() const;

private:
    qreal ixp; // inner rect
    qreal iyp;
    qreal iw;
    qreal ih;

//    qreal oxp; // outer rect
//    qreal oyp;
//    qreal ow;
//    qreal oh;
};

#endif
