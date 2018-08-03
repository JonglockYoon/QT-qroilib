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

#include "geviport.h"
#include "bootstrapregisters.h"
#include "writereghelper.h"

#include <QHostAddress>

using namespace Qroilib;

void GevIPort::addReceiver(int channel, const QHostAddress &address, quint16 port)
{
    writeRegisters(
    { Jgv::Gvcp::ADDRESS_VALUE{Jgv::Gvcp::BootstrapRegisters::streamChannelTranspose(channel, Jgv::Gvcp::BootstrapAddress::StreamChannelDestinationAddress), address.toIPv4Address()},
      Jgv::Gvcp::ADDRESS_VALUE{Jgv::Gvcp::BootstrapRegisters::streamChannelTranspose(channel, Jgv::Gvcp::BootstrapAddress::StreamChannelPort), port} }
                );
}

void GevIPort::read(quint8 *pBuffer, qint64 address, qint64 length)
{
    const quint8 *memory = readMemory(address, length);
    if (memory == nullptr) {
        qDebug("GevIport failed to read address 0x%llX size %lld", static_cast<quint64>(address), length);
        return;
    }
    std::memcpy(pBuffer, memory, length);

}

void GevIPort::write(quint8 *pBuffer, qint64 address, qint64 length)
{
    if (!writeMemory(address, pBuffer, length)) {
        qDebug("GevIport failed to write address 0x%llX size %lld", static_cast<quint64>(address), length);
    }
}
