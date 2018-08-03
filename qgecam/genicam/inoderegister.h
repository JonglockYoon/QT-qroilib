/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                                  *
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

#ifndef REGISTERINODE_H
#define REGISTERINODE_H

#include "inode.h"

namespace Jgv {

namespace GenICam {

namespace IPort {
class Interface;
}

namespace InodeRegister {

class ObjectPrivate;
class Object : public Inode::Object
{

protected:
    Object(ObjectPrivate &dd);

public:
    virtual ~Object();

public:
    virtual void prepare(InterfaceBuilder &builder, const XMLHelper &helper);
    QStringList getInvalidatorFeatureNames() const;
    int getPollingTime() const;
    bool isWritable() const;

protected:
    /*!
     * \brief data
     * Obtient un pointeur sur uchar, pointant sur le cache local du registre sous-jacent.
     * \return Un pointeur sur le cache local du registre.
     */
    uchar *data();
    /*!
     * \brief dataSize
     * Obtient la taille du cache.
     * \return La taille du cache
     */
    qint64 dataSize() const;
    /*!
     * \brief updateData
     * Demande la mise à jour de l'iport avec la valeur du cache.
     */
    void updateData();
    /*!
     * \brief address
     * Obtient l'adresse du registre sur le que pointe linoderegister.
     * \return L'adresse du registre.
     */
    qint64 address() const;

private:
    Q_DECLARE_PRIVATE(Object)

}; // Interface

} // namespace InodeRegister

} // namespace GenICam

} // namespace Jgv

#endif // REGISTERINODE_H
