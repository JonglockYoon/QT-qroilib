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

#include "discoveryhelper.h"

#include <QtWidgets>
#include <QtNetwork>

#ifdef Q_OS_WIN
//#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace Jgv::Gvcp;

DiscoveryCmdHelper::DiscoveryCmdHelper(uint16_t length)
    : HeaderCmdHelper(GVCP_CMD_DISCOVERY, length),
      cmd(reinterpret_cast<DISCOVERY_CMD * const>(data))
{}

void DiscoveryCmdHelper::allowBroadcastAck(bool allow)
{
    cmd->header.flag = allow ? (0x10 | cmd->header.flag) : (0xEF & cmd->header.flag);
}


DiscoveryAckHelper::DiscoveryAckHelper(uint16_t length)
    : HeaderAckHelper(GVCP_ACK_DISCOVERY, length),
      ack(reinterpret_cast<DISCOVERY_ACK * const>(data))
{}

void DiscoveryAckHelper::setSpecVersionMajor(uint16_t value)
{
    ack->specVersionMajor = htons(value);
}

void DiscoveryAckHelper::setSpecVersionMinor(uint16_t value)
{
    ack->specVersionMinor = htons(value);
}

void DiscoveryAckHelper::setDeviceMode(uint32_t mode)
{
    ack->deviceMode = htonl(mode);
}

void DiscoveryAckHelper::setDeviceMACaddressHigh(uint16_t mac)
{
    ack->deviceMACAddressHigh = htons(mac);
}

void DiscoveryAckHelper::setDeviceMACaddressLow(uint32_t mac)
{
    ack->deviceMACAddressLow = htonl(mac);
}

void DiscoveryAckHelper::setIPconfigOptions(uint32_t options)
{
    ack->ipConfigOptions = htonl(options);
}

void DiscoveryAckHelper::setIPconfigCurrent(uint32_t current)
{
    ack->ipConfigCurrent = htonl(current);
}

void DiscoveryAckHelper::setCurrentIP(uint32_t IP)
{
    ack->currentIP = htonl(IP);
}

void DiscoveryAckHelper::setCurrentSubnetMask(uint32_t mask)
{
    ack->currentSubnetMask = htonl(mask);
}

void DiscoveryAckHelper::setDefaultGateway(uint32_t gateway)
{
    ack->defaultGateway = htonl(gateway);
}

void DiscoveryAckHelper::setManufacturerName(const char *name)
{
    std::memcpy(ack->manufacturerName, name, sizeof(ack->manufacturerName));
}

void DiscoveryAckHelper::setModelName(const char *name)
{
    std::memcpy(ack->modelName, name, sizeof(ack->modelName));
}

void DiscoveryAckHelper::setDeviceVersion(const char *name)
{
    std::memcpy(ack->deviceVersion, name, sizeof(ack->deviceVersion));
}

void DiscoveryAckHelper::setManufacturerInformation(const char *name)
{
    std::memcpy(ack->manufacturerSpecificInformation, name, sizeof(ack->manufacturerSpecificInformation));
}

void DiscoveryAckHelper::setSerialNumber(const char *name)
{
    std::memcpy(ack->serialNumber, name, sizeof(ack->serialNumber));
}

void DiscoveryAckHelper::setUserName(const char *name)
{
    std::memcpy(ack->userDefinedName, name, sizeof(ack->userDefinedName));
}

uint16_t DiscoveryAckHelper::specVersionMajor(const DISCOVERY_ACK &ack)
{
    return ntohs(ack.specVersionMajor);
}

uint16_t DiscoveryAckHelper::specVersionMinor(const DISCOVERY_ACK &ack)
{
    return ntohs(ack.specVersionMinor);
}

uint32_t DiscoveryAckHelper::deviceMode(const DISCOVERY_ACK &ack)
{
    return ntohl(ack.deviceMode);
}

uint8_t DiscoveryAckHelper::deviceClass(uint32_t deviceMode)
{
    return static_cast<uint8_t>( (0x70000000 & deviceMode) >> 28 );
}

uint16_t DiscoveryAckHelper::deviceMACaddressHigh(const DISCOVERY_ACK &ack)
{
    return ntohs(ack.deviceMACAddressHigh);
}

uint32_t DiscoveryAckHelper::deviceMACaddressLow(const DISCOVERY_ACK &ack)
{
    return ntohl(ack.deviceMACAddressLow);
}

uint32_t DiscoveryAckHelper::IPconfigOptions(const DISCOVERY_ACK &ack)
{
    return ntohl(ack.ipConfigOptions);
}

uint32_t DiscoveryAckHelper::IPconfigCurrent(const DISCOVERY_ACK &ack)
{
    return ntohl(ack.ipConfigCurrent);
}

uint32_t DiscoveryAckHelper::currentIP(const DISCOVERY_ACK &ack)
{
    return ntohl(ack.currentIP);
}

uint32_t DiscoveryAckHelper::currentSubnetMask(const DISCOVERY_ACK &ack)
{
    return ntohl(ack.currentSubnetMask);
}

uint32_t DiscoveryAckHelper::defaultGateway(const DISCOVERY_ACK &ack)
{
    return ntohl(ack.defaultGateway);
}

const char *DiscoveryAckHelper::manufacturerName(const DISCOVERY_ACK &ack)
{
    return reinterpret_cast<const char *>(ack.manufacturerName);
}

const char *DiscoveryAckHelper::modelName(const DISCOVERY_ACK &ack)
{
    return reinterpret_cast<const char *>(ack.modelName);
}

const char *DiscoveryAckHelper::deviceVersion(const DISCOVERY_ACK &ack)
{
    return reinterpret_cast<const char *>(ack.deviceVersion);
}

const char *DiscoveryAckHelper::manufacturerSpecificInformation(const DISCOVERY_ACK &ack)
{
    return reinterpret_cast<const char *>(ack.manufacturerSpecificInformation);
}

const char *DiscoveryAckHelper::serialNumber(const DISCOVERY_ACK &ack)
{
    return reinterpret_cast<const char *>(ack.serialNumber);
}

const char *DiscoveryAckHelper::userDefinedName(const DISCOVERY_ACK &ack)
{
    return reinterpret_cast<const char *>(ack.userDefinedName);
}


