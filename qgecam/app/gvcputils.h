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

#ifndef GVCPUTILS_H
#define GVCPUTILS_H

#include "qtpimpl.hpp"
#include <QObject>
#include <QScopedPointer>
#include "bootstrapregisters.h"

class QNetworkAddressEntry;
class QHostAddress;

namespace Jgv {
namespace Gvcp {
struct DISCOVERY_ACK;
}
}

namespace Qroilib {

class GvcpUtilsPrivate;
class GvcpUtils : public QObject
{
    Q_OBJECT
public:
    GvcpUtils(QObject *parent = nullptr);
    ~GvcpUtils();

    void setController(const QNetworkAddressEntry &interface);
    QNetworkAddressEntry controller() const;
    QHostAddress controllerAddress() const;

    void setTransmitterAck(const Jgv::Gvcp::DISCOVERY_ACK &ack);
    Jgv::Gvcp::DISCOVERY_ACK transmitterAck() const;
    QHostAddress transmitterAddress() const;

    void listenForBroadcast();
    void stopBroadcastListener();
    void discover(const QHostAddress &peerIP);
    void forceIP(quint64 mac, const QHostAddress &newIP, const QHostAddress &newNetmask, const QHostAddress &newGateway);

    bool monitorTransmitter();
    void releaseTransmitter();
    Jgv::Gvcp::CCPPrivilege transmitterAccessMode();
    QByteArray readXmlFilenameFromDevice();
    void setXmlFilename(const QByteArray &name);
    QByteArray xmlFilename() const;
    QByteArray readXmlFileFromDevice();
    void setXmlFile(const QByteArray &file);
    QByteArray xmlFile() const;
    void setWantControl(bool wanted);
    bool wantControl() const;


signals:
    void newDeviceDiscovered(const Jgv::Gvcp::DISCOVERY_ACK &device);
    void forceIpSucces();

private:
    const QScopedPointer<GvcpUtilsPrivate> d_ptr;
    Q_DECLARE_PRIVATE(GvcpUtils)

};

} // namespace Qroilib

#endif // GVCPUTILS_H
