#include "qpatternroif.h"

QPatternRoiF::QPatternRoiF()
{
    ixp = 0;
    iyp = 0;
    iw = 0;
    ih = 0;
}
QPatternRoiF::~QPatternRoiF() {}

QPatternRoiF::QPatternRoiF(const QRectF &r)
    : ixp(r.x()), iyp(r.y()), iw(r.width()), ih(r.height())
{
}

qreal QPatternRoiF::x() const
{ return ixp; }

qreal QPatternRoiF::y() const
{ return iyp; }

qreal QPatternRoiF::width() const
{ return  iw; }

qreal QPatternRoiF::height() const
{ return  ih; }

void QPatternRoiF::setRect(const QRectF &r)
{
    ixp = r.x();
    iyp = r.y();
    iw = r.width();
    ih = r.height();
}

QRectF QPatternRoiF::getRect() const
{
    return QRectF(ixp, iyp, iw, ih);
}
