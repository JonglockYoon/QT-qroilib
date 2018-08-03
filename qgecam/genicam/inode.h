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

#ifndef INODE_H
#define INODE_H

#include <QScopedPointer>
#include <QString>
#include <QMetaType>

namespace Jgv {

namespace GenICam {

class Interface;
class InterfaceBuilder;
class XMLHelper;

namespace IPort {
class Interface;
}

namespace Integer {
class Interface;
}

namespace Inode {

class ObjectPrivate;
class Object
{

public:
    Object(QSharedPointer<IPort::Interface> iport);
    virtual ~Object();

protected:
    Object(ObjectPrivate &dd);

public:
    virtual void prepare(InterfaceBuilder &builder, const XMLHelper &helper);

    const QString &typeString() const;
    const QString &toolTip() const;
    const QString &description() const;
    const QString &displayName() const;
    const QString &featureName() const;
    const QString &visibility() const;

    /*!
     * \brief Interface::setInodeToInvalidate
     * Ajoute un inode à invalider lors de la modification de la valeur de l'inode.
     * \param inode Pointeur faible vers l'inode à invalider
     */
    void setInodeToInvalidate(QWeakPointer<Inode::Object> inode);
    /*!
     * \brief Interface::invalidateNames
     * Obtient la liste des noms des inodes que l'inode courant invalide à chaque midification de la valeur.
     * \return Une liste de featureNames
     */
    QStringList invalidateNames() const;
    /*!
     * \brief Interface::getInvalidatorNames
     * Obtient la liste des noms des inodes qui invalident la valeur de l'inode courant.
     * \return Une liste de featureNames
     */
    virtual QStringList getInvalidatorFeatureNames() const;
    /*!
     * \brief Interface::invalidate
     * Demande l'invalidation du cache de l'inode courant.
     */
    virtual void invalidate() = 0;


    // Interface MVC

    /*!
     * \brief Interface::setHierarchy
     * Fixe la hiérarchie de l'inode pour l'utilisition du modèle MVC.
     * \param row Un entier représentant le niveau de filiation par rapport au père.
     * \param parent le père de l'inode courant.
     */
    void setHierarchy(int row, QWeakPointer<Inode::Object> parent);
    /*!
     * \brief Interface::row
     * Obtient l'indice de filiation de l'inode courant par rapport à son père.
     * \return
     */
    int row() const;
    /*!
     * \brief Interface::parent
     * Obtient le père de l'inode courant.
     * \return Un pointeur faible sur l'inode père
     */
    QWeakPointer<Object> parent() const;
    /*!
     * \brief Interface::getChild
     * Obtient le fils de l'inode courant à l'indice de filiation.
     * \param raw L'indice de filiation de l'inode enfant.
     * \return Un pointeur faible sur le fils de l'inode.
     */
    virtual QWeakPointer<Object> getChild(int raw) const;
    /*!
     * \brief Interface::haveChild
     * Détermine si l'inode courant possède au moins un enfant.
     * \return true Si l'inode possède au moins un enfant.
     */
    virtual bool haveChild() const;
    /*!
     * \brief Interface::childCount
     * Obtient le nombre d'enfant de l'inode courant.
     * \return Le nombre d'enfant de l'inode courant.
     */
    virtual int childCount() const;
    /*!
     * \brief Interface::getPollingTime
     * Obtient le temps en millisecondes auquel il faut régulièrement mettre à jour la valeur de l'inode.
     * \return Le temps de polling de la valeur de l'inode, -1 si il n'y a pas de polling.
     */
    virtual int getPollingTime() const;
    /*!
     * \brief Interface::isImplemented
     * Vrai si l'inode est effectivement implémenté sur le device.
     * \return Si la valeur de l'inode est présent sur le device.
     */
    bool isImplemented() const;
    /*!
     * \brief Interface::isAvailable
     * Vrai si l'inode est effectivement disponible au moment de l'appel.
     * \return Si la valeur de l'inode est disponible au moment de l'appel.
     */
    bool isAvailable() const;
    /*!
     * \brief Interface::isLocked
     * Obtient si l'inode est actuellement vérrouillé en écriture.
     * \return Si l'inode est actuellement verrouillé en écriture.
     */
    bool isLocked() const;

    qint64 error() const;
    QString errrorString(qint64 error) const;

    /*!
     * \brief Interface::isWritable
     * Détermine si l'on peut modifier la valeur de l'inode.
     * \return Si l'accès en écriture est disponible.
     */
    virtual bool isWritable() const = 0;
    /*!
     * \brief Interface::getVariant
     * Obtient la valeur de l'inode.
     * \return Un variant portant le type et la valeur de l'inode.
     */
    virtual QVariant getVariant() = 0;
    /*!
     * \brief Interface::interface
     * Obtient un pointeur sur l'interface IInterface de l'inode.
     * \return Le pointeur sur IINterface::interface.
     */
    virtual Interface *interface() = 0;
    /*!
     * \brief Interface::constInterface
     * Obtient un pointeur const sur l'interface IInterface de l'inode.
     * \return Le pointeur const sur IInterface::interface.
     */
    virtual const Interface *constInterface() const = 0;

protected:
    const QScopedPointer<ObjectPrivate> d_ptr;

private:
    Q_DISABLE_COPY(Object)
    Q_DECLARE_PRIVATE(Object)

};


struct Ptr {
    Ptr() = default;
    Ptr(Inode::Object *ptr): data(ptr) {}
    Inode::Object * const data = nullptr;
};

} // namespace Inode

} // namespace GenICam

} // namespace Jgv

// Permet la sérialisation dans un QVariant
Q_DECLARE_METATYPE(Jgv::GenICam::Inode::Ptr)

#endif // INODE_H
