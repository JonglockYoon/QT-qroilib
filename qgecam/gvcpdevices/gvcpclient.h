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

#ifndef GVCPCLIENT_H
#define GVCPCLIENT_H

#include <memory>
#include <vector>
#include <QObject>

namespace Jgv {

namespace Gvcp {

class HeaderCmdHelper;

struct ADDRESS_VALUE;
using AddrValPairList = std::vector<ADDRESS_VALUE>;
enum class BootstrapAddress: uint32_t;
enum class BootstrapBlockSize: uint16_t;

struct Datation {
    uint64_t timestamp;
    uint64_t dateMin;
    uint64_t dateMax;
};


class ClientPrivate;
class Client : public QObject
{
    Q_OBJECT

public:
    Client();
    virtual ~Client();

    bool controleDevice(uint32_t controllerIP, uint32_t deviceIP);
    bool monitorDevice(uint32_t monitorIP, uint32_t deviceIP);
    void releaseDevice();

    std::vector<uint32_t> readRegisters(const std::vector<uint32_t> &addresses);
    bool writeRegisters(const AddrValPairList &values);

    const uint8_t *readMemory(uint32_t address, uint16_t count);
    bool writeMemory(uint32_t address, const uint8_t *data, uint32_t size);

    std::string xmlFile();
    std::string xmlFilename();

    Datation getDatation();

    //HeaderCmdHelper *pCmd;
    //uint16_t nAckType;

private:
    const std::unique_ptr<ClientPrivate> d_ptr;
    inline ClientPrivate *d_func() { return d_ptr.get(); }
    inline const ClientPrivate *d_func() const { return d_ptr.get(); }

};

} // namespace Gvcp

} // namespace Jgv

#endif // GVCPCLIENT_H
