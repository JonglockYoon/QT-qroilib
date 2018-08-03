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

#ifndef QTPIMPL_HPP
#define QTPIMPL_HPP

#include <QScopedPointer>

#define QT_PIMPL(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(p_impl.data()); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(p_impl.data()); } \
    Class(const Class &) = delete; \
    Class &operator=(const Class &) = delete;

#define QT_BASE_PIMPL(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(p_impl.data()); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(p_impl.data()); } \
    Class(const Class &) = delete; \
    Class &operator=(const Class &) = delete; \
    protected: \
    const QScopedPointer<Class##Private> p_impl;

#define QT_IMPL(Class) Class##Private * const d = d_func()

#endif // QTPIMPL_HPP
