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

#ifndef GVCPUTILS_P_H
#define GVCPUTILS_P_H

#include "gvcpdiscoverer.h"
#include "discoveryhelper.h"
#include "gvcpclient.h"

#include <QMetaType>
#include <QNetworkAddressEntry>
#include <memory>

namespace Qroilib {

class Wrapper : public QObject, public Jgv::Gvcp::Discoverer::IListener
{
    Q_OBJECT

public:
    void deviceDiscovered(const Jgv::Gvcp::DISCOVERY_ACK &device)
    {
        emit newDeviceDiscovered(device);
    }
    void forceIpSucces()
    {
        emit forceIpFinished();
    }

signals:
    void newDeviceDiscovered(const Jgv::Gvcp::DISCOVERY_ACK &device);
    void forceIpFinished();
};

class GvcpUtilsPrivate
{
public:
    Jgv::Gvcp::GvcpDiscoverer discoverer;
    Jgv::Gvcp::Client gvcp;
    std::shared_ptr<Wrapper> wrapper;
    QNetworkAddressEntry controller;
    Jgv::Gvcp::DISCOVERY_ACK transmitter;

    QByteArray xmlFilename;
    QByteArray xmlFile;
    bool wantcontrol = true;
};

} // namespace Qroilib

Q_DECLARE_METATYPE(Jgv::Gvcp::DISCOVERY_ACK)



#endif // GVCPUTILS_P_H
