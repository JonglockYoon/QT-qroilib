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

#ifndef TIMESTAMPDATE_P_H
#define TIMESTAMPDATE_P_H

#include <mutex>
#include <list>

namespace Jgv
{

namespace Gvsp
{

struct TimeData {
    uint64_t timestamp;
    uint64_t min;
    uint64_t max;
};

class TimestampDatePrivate
{
public:
    long double a = 1;
    long double b = 0;
    long double x = 0;
    long double y = 0;

    const long double k = 1024;
    std::list<TimeData> datas;

    std::mutex read_mutex;
    std::mutex write_mutex;
    void updateByRegression();

}; // class TimestampDatePrivate

} // namespace Gvsp

} // namespace Jgv

#endif // TIMESTAMPDATE_P_H
