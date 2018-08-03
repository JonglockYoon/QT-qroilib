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

#ifndef DISCOVERYHELPER_H
#define DISCOVERYHELPER_H

#include "headerhelper.h"

namespace Jgv {

namespace Gvcp {

struct DISCOVERY_CMD
{
    CMD_HEADER header;
};

struct DISCOVERY_ACK
{
    ACK_HEADER header;
    uint16_t specVersionMajor;
    uint16_t specVersionMinor;
    uint32_t deviceMode;
    uint16_t reserved1;
    uint16_t deviceMACAddressHigh;
    uint32_t deviceMACAddressLow;
    uint32_t ipConfigOptions;
    uint32_t ipConfigCurrent;
    uint32_t reserved2[3];
    uint32_t currentIP;
    uint32_t reserved3[3];
    uint32_t currentSubnetMask;
    uint32_t reserved4[3];
    uint32_t defaultGateway;
    char manufacturerName[32];
    char modelName[32];
    char deviceVersion[32];
    char manufacturerSpecificInformation[48];
    char serialNumber[16];
    char userDefinedName[16];
};



constexpr uint16_t GVCP_CMD_DISCOVERY                = 0x0002;
constexpr uint16_t GVCP_ACK_DISCOVERY                = 0x0003;
constexpr int DISCOVERY_CMD_LENGTH = sizeof(DISCOVERY_CMD) - sizeof(CMD_HEADER);
constexpr int DISCOVERY_ACK_LENGTH = sizeof(DISCOVERY_ACK) - sizeof(ACK_HEADER);

class DiscoveryCmdHelper final : public HeaderCmdHelper
{
    DISCOVERY_CMD * const cmd;

public:
    DiscoveryCmdHelper(uint16_t length);
    virtual ~DiscoveryCmdHelper() = default;

    void allowBroadcastAck(bool allow);

};

class DiscoveryAckHelper final : public HeaderAckHelper
{
    DISCOVERY_ACK * const ack;

public:
    DiscoveryAckHelper(uint16_t length);
    virtual ~DiscoveryAckHelper() = default;

    void setSpecVersionMajor(uint16_t value);
    void setSpecVersionMinor(uint16_t value);
    void setDeviceMode(uint32_t mode);
    void setDeviceMACaddressHigh(uint16_t mac);
    void setDeviceMACaddressLow(uint32_t mac);
    void setIPconfigOptions(uint32_t options);
    void setIPconfigCurrent(uint32_t current);
    void setCurrentIP(uint32_t IP);
    void setCurrentSubnetMask(uint32_t mask);
    void setDefaultGateway(uint32_t gateway);
    void setManufacturerName(const char *name);
    void setModelName(const char *name);
    void setDeviceVersion(const char *name);
    void setManufacturerInformation(const char *name);
    void setSerialNumber(const char *name);
    void setUserName(const char *name);

    static uint16_t specVersionMajor(const DISCOVERY_ACK &ack);
    static uint16_t specVersionMinor(const DISCOVERY_ACK &ack);
    static uint32_t deviceMode(const DISCOVERY_ACK &ack);

    static uint8_t deviceClass(uint32_t deviceMode);
    static uint16_t deviceMACaddressHigh(const DISCOVERY_ACK &ack);
    static uint32_t deviceMACaddressLow(const DISCOVERY_ACK &ack);
    static uint32_t IPconfigOptions(const DISCOVERY_ACK &ack);
    static uint32_t IPconfigCurrent(const DISCOVERY_ACK &ack);
    static uint32_t currentIP(const DISCOVERY_ACK &ack);
    static uint32_t currentSubnetMask(const DISCOVERY_ACK &ack);
    static uint32_t defaultGateway(const DISCOVERY_ACK &ack);
    static const char *manufacturerName(const DISCOVERY_ACK &ack);
    static const char *modelName(const DISCOVERY_ACK &ack);
    static const char *deviceVersion(const DISCOVERY_ACK &ack);
    static const char *manufacturerSpecificInformation(const DISCOVERY_ACK &ack);
    static const char *serialNumber(const DISCOVERY_ACK &ack);
    static const char *userDefinedName(const DISCOVERY_ACK &ack);
};

} // namespace Gvcp

} // namespace Jgv

#endif // READREGHELPER_H
