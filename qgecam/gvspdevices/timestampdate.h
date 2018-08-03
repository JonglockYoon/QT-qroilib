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

#ifndef TIMESTAMPDATE_H
#define TIMESTAMPDATE_H

#include <memory>

namespace Jgv
{

namespace Gvsp
{

class TimestampDatePrivate;
class TimestampDate final
{
public:
    TimestampDate();
    ~TimestampDate();

    void push(uint64_t timestamp, uint64_t dateMin, uint64_t dateMax) noexcept;
    uint64_t getDate(uint64_t timestamp) noexcept;

private:
    const std::unique_ptr<TimestampDatePrivate> d_ptr;
    inline TimestampDatePrivate *d_func() noexcept { return d_ptr.get(); }
    inline const TimestampDatePrivate *d_func() const noexcept { return d_ptr.get(); }

}; // clas TimestampDate

} // namespace Gvcp

} // namespace Jgv

#endif // TIMESTAMPDATE_H
