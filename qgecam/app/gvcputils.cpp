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

#include "gvcputils.h"
#include "gvcputils_p.h"
#include "bootstrapregisters.h"

#include <QHostAddress>
#include <QByteArray>

using namespace Qroilib;

GvcpUtils::GvcpUtils(QObject *parent)
    : QObject(parent), d_ptr(new GvcpUtilsPrivate)
{
    qRegisterMetaType<Jgv::Gvcp::DISCOVERY_ACK>();
    d_ptr->wrapper = std::make_shared<Wrapper>();
    connect(d_ptr->wrapper.get(), &Wrapper::newDeviceDiscovered, this, &GvcpUtils::newDeviceDiscovered);
    connect(d_ptr->wrapper.get(), &Wrapper::forceIpFinished, this, &GvcpUtils::forceIpSucces);
    d_ptr->discoverer.addListener(d_ptr->wrapper);
}

GvcpUtils::~GvcpUtils()
{}

void GvcpUtils::setController(const QNetworkAddressEntry &interface)
{
    Q_D(GvcpUtils);
    d->controller = interface;
}

QNetworkAddressEntry GvcpUtils::controller() const
{
    Q_D(const GvcpUtils);
    return d->controller;
}

QHostAddress GvcpUtils::controllerAddress() const
{
    Q_D(const GvcpUtils);
    return d->controller.ip();
}

void GvcpUtils::setTransmitterAck(const Jgv::Gvcp::DISCOVERY_ACK &ack)
{
    Q_D(GvcpUtils);
    d->transmitter = ack;
}

Jgv::Gvcp::DISCOVERY_ACK GvcpUtils::transmitterAck() const
{
    Q_D(const GvcpUtils);
    return d->transmitter;
}

QHostAddress GvcpUtils::transmitterAddress() const
{
    Q_D(const GvcpUtils);
    return QHostAddress(Jgv::Gvcp::DiscoveryAckHelper::currentIP(d->transmitter));
}

void GvcpUtils::listenForBroadcast()
{
    Q_D(GvcpUtils);
    d->discoverer.listen(d->controller.ip().toIPv4Address());
}

void GvcpUtils::stopBroadcastListener()
{
    Q_D(GvcpUtils);
    d->discoverer.stop();
}

void GvcpUtils::discover(const QHostAddress &peerIP)
{
    Q_D(GvcpUtils);
    d->discoverer.discover(peerIP.toIPv4Address());
}

void GvcpUtils::forceIP(quint64 mac, const QHostAddress &newIP, const QHostAddress &newNetmask, const QHostAddress &newGateway)
{
    Q_D(GvcpUtils);
    d->discoverer.forceIP(mac, newIP.toIPv4Address(), newNetmask.toIPv4Address(), newGateway.toIPv4Address());
}

bool GvcpUtils::monitorTransmitter()
{
    Q_D(GvcpUtils);
    return d->gvcp.monitorDevice(d->controller.ip().toIPv4Address(), Jgv::Gvcp::DiscoveryAckHelper::currentIP(d->transmitter));
}

void GvcpUtils::releaseTransmitter()
{
    Q_D(GvcpUtils);
    d->gvcp.releaseDevice();
}

#if 1 // jlyoon test

Jgv::Gvcp::CCPPrivilege GvcpUtils::transmitterAccessMode()
{
    Q_D(GvcpUtils);
    std::vector<uint32_t> regs = d->gvcp.readRegisters({Jgv::Gvcp::enumType(Jgv::Gvcp::BootstrapAddress::ControlChannelPrivilege)});
    if (regs.size() > 0) {
        return Jgv::Gvcp::CCP::privilegeFromCCP(regs.at(0));
    }
    return Jgv::Gvcp::CCPPrivilege::AccesDenied;
}

#endif

QByteArray GvcpUtils::readXmlFilenameFromDevice()
{
    Q_D(GvcpUtils);

#if 1 // jlyoon test
    const uint8_t *p = d->gvcp.readMemory(Jgv::Gvcp::enumType(Jgv::Gvcp::BootstrapAddress::FirstURL), Jgv::Gvcp::enumType(Jgv::Gvcp::BootstrapBlockSize::FirstURL));
    if (p == nullptr) {
        qWarning("GvcpUtils: read firstURL failed");
        return QByteArray();
    }

    QByteArray firstUrl(reinterpret_cast<const char *>(p), Jgv::Gvcp::enumType(Jgv::Gvcp::BootstrapBlockSize::FirstURL));
    const QByteArray begin = "Local:";
    if (firstUrl.startsWith(begin)) {
        // on split les 3 champs
        QList<QByteArray> firstURLFields = firstUrl.split(';');
        if (firstURLFields.size() == 3) {
            const int s = firstURLFields[0].size();
            return firstURLFields[0].right(s - begin.length());
        }
    }
    else {
        qWarning("GvcpUtils: failed to decode firstUrl %s", firstUrl.constData());
    }
#endif
    return QByteArray();
}

void GvcpUtils::setXmlFilename(const QByteArray &name)
{
    Q_D(GvcpUtils);
    d->xmlFilename = name;
}

QByteArray GvcpUtils::xmlFilename() const
{
    Q_D(const GvcpUtils);
    return d->xmlFilename;
}

QByteArray GvcpUtils::readXmlFileFromDevice()
{
    Q_D(GvcpUtils);

    // firstUrl
    const uint8_t *p = d->gvcp.readMemory(Jgv::Gvcp::enumType(Jgv::Gvcp::BootstrapAddress::FirstURL), Jgv::Gvcp::enumType(Jgv::Gvcp::BootstrapBlockSize::FirstURL));
    if (p == nullptr) {
        qWarning("GvcpUtils: read firstURL failed");
        return QByteArray();
    }
    QByteArray url(reinterpret_cast<const char *>(p), static_cast<uint16_t>(Jgv::Gvcp::BootstrapBlockSize::FirstURL));

    QByteArray content;
    if (url.startsWith("Local:")) {
        QList<QByteArray> firstURLFields = url.split(';');
        if (firstURLFields.count() == 3) {
            bool ok;
            quint32 address = firstURLFields.at(1).toUInt(&ok, 16);
            if (ok) {
                quint32 size = firstURLFields.at(2).toUInt(&ok, 16);
                if (ok) {
                    // le payload max
                    const int MEMREADPayload = 0x200;
                    // le nombre de packets identiques
                    const int packetCount = size / MEMREADPayload;
                    // la taille du dernier packet
                    const int lastPacketSize = Jgv::Gvcp::BootstrapRegisters::align(size - (packetCount * MEMREADPayload));
                    // obtient tous les packets de taille identique
                    for (int i=0; i<packetCount; ++i)  {
                        content.append(reinterpret_cast<const char *>(d->gvcp.readMemory(address + i * MEMREADPayload, MEMREADPayload)),
                                       MEMREADPayload);
                    }
                    // fini avec le dernier packet
                    content.append(reinterpret_cast<const char *>(d->gvcp.readMemory(address + packetCount * MEMREADPayload, lastPacketSize))
                                   , lastPacketSize);
                }
            }
        }
    }
    return content;
}

void GvcpUtils::setXmlFile(const QByteArray &file)
{
    Q_D(GvcpUtils);
    d->xmlFile = file;
}

QByteArray GvcpUtils::xmlFile() const
{
    Q_D(const GvcpUtils);
    return d->xmlFile;
}

void GvcpUtils::setWantControl(bool wanted)
{
    Q_D(GvcpUtils);
    d->wantcontrol = wanted;
}

bool GvcpUtils::wantControl() const
{
    Q_D(const GvcpUtils);
    return d->wantcontrol;
}

