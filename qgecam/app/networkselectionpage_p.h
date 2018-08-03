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

#ifndef NETWORKSELECTIONPAGE_P_H
#define NETWORKSELECTIONPAGE_P_H

#include "jgvwizardpage_p.h"
#include <QRadioButton>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

class QFormLayout;
class QRadioButton;

namespace Qroilib {

class NetworkInterfaceRadioButton : public QRadioButton
{
    QNetworkAddressEntry m_address;
    QNetworkInterface m_ni;
public:
    explicit NetworkInterfaceRadioButton(const QNetworkAddressEntry &address, const QNetworkInterface &ni)
        : QRadioButton(), m_address(address), m_ni(ni)
    {
        setText(QString("%0 %1").arg(address.ip().toString()).arg(ni.name()));
    }

    QNetworkAddressEntry address() const {return m_address;}

    quint32 ip() const {return m_address.ip().toIPv4Address();}
    quint32 netmask() const {return m_address.netmask().toIPv4Address();}
    quint32 broadcast() const {return m_address.broadcast().toIPv4Address();}
    int index() const {return m_ni.index();}

};

class NetworkSelectionPagePrivate : public JgvWizardPagePrivate
{
public:
    QFormLayout *mainLayout = nullptr;
    QList<NetworkInterfaceRadioButton *> interfaces;
};

} // namespace Jiguiviou

#endif // NETWORKSELECTIONPAGE_P_H
