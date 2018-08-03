/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                             *
 *   cyril.baletaud@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "timestampdate.h"
#include "timestampdate_p.h"

#include <cmath>

#define D(Class) Class##Private * const d = d_func()

using namespace Jgv::Gvsp;

void TimestampDatePrivate::updateByRegression()
{
    auto it = datas.cbegin();
    if (it == datas.cend()) {
        return;
    }

    long double xsomme = 0.;
    long double ysomme = 0.;

    // on fait une translation de repère, pour ne pas déborder
    const long double x0 = (*it).timestamp;
    const long double y0 = (*it).min;

    for (it = datas.cbegin(); it != datas.cend(); ++it) {
        const long double x = (*it).timestamp - x0;
        const long double y1 = (*it).min - y0;
        const long double y2 = (*it).max - y0;
        xsomme += x;
        ysomme += (y1 + y2);
    }
    const long double moyX = xsomme / datas.size();
    const long double moyY = ysomme / (datas.size() * 2);

    long double nu = 0;
    long double de = 0;

    for (it = datas.cbegin(); it != datas.cend(); ++it) {
        const long double x = (*it).timestamp - x0;
        const long double y1 = (*it).min - y0;
        const long double y2 = (*it).max - y0;

        nu += (x - moyX) / k * (y1 - moyY) / k;
        nu += (x - moyX) / k * (y2 - moyY) / k;
        de += ((x - moyX) / k * (x - moyX) / k);
        de += ((x - moyX) / k * (x - moyX) / k);
    }

    const long double aa = nu / de;
    const long double bb = moyY - aa * static_cast<long double>(moyX);

    std::lock_guard<std::mutex> lock(read_mutex);
    a = aa;
    b = bb;
    x = x0;
    y = y0;
}

TimestampDate::TimestampDate()
    : d_ptr(new TimestampDatePrivate)
{}

TimestampDate::~TimestampDate()
{}

void TimestampDate::push(uint64_t timestamp, uint64_t dateMin, uint64_t dateMax) noexcept
{
    D(TimestampDate);

    std::lock_guard<std::mutex> lock(d->write_mutex);
    // on filtre, en supprimant les données dont l'écart de mesure est le plus grand
    while (d->datas.size() > 15) {
        auto it = d->datas.cbegin();
        auto toRemove = it;
        int delta = 0;
        for (; it != d->datas.cend(); ++it) {
            int d = (*it).max - (*it).min;
            if (d > delta) {
                delta = d;
                toRemove = it;
            }
        }
        //d->datas.erase(toRemove);
    }

    d->datas.push_back(TimeData {timestamp, dateMin, dateMax});

    if (d->datas.size() > 2) {
        d->updateByRegression();
    }
}

uint64_t TimestampDate::getDate(uint64_t timestamp) noexcept
{
    D(TimestampDate);

    std::lock_guard<std::mutex> lock(d->read_mutex);
    long double ts = (static_cast<long double>(timestamp) - d->x) * d->a;
    ts += (d->y + d->b);

    return std::llround(ts);
}











