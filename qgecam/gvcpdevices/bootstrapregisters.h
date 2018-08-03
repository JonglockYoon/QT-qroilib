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

#ifndef BOOTSTRAPREGISTERS_H
#define BOOTSTRAPREGISTERS_H

#include "gvcp.h"

#include <stdint.h>
#include <string>

namespace Jgv {

namespace Gvcp {

enum class BootstrapAddress: uint32_t {
    Version                         = 0x0000,
    DeviceMode                      = 0x0004,
    DeviceMacAddressHigh_0          = 0x0008,
    DeviceMacAddressLow_0           = 0x000C,
    SupportedIPconfiguration_0      = 0x0010,
    CurrentIPconfiguration_0        = 0x0014,
    CurrentIPAddress_0              = 0x0024,
    CurrentSubnetMask_0             = 0x0034,
    CurrentDefaultGateway_0         = 0x0044,
    ManufacturerName                = 0x0048,
    ModelName                       = 0x0068,
    DeviceVersion                   = 0x0088,
    ManufacturerInfo                = 0x00A8,
    SerialNumber                    = 0x00D8,
    UserDefinedName                 = 0x00E8,
    FirstURL                        = 0x0200,
    SecondURL                       = 0x0400,
    NumberOfNetworkInterfaces       = 0x0600,
    PersistentIPAddress_0           = 0x064C,
    PersistentSubnetMask_0          = 0x065C,
    PersistentDefaultGateway_0      = 0x066C,
    LinkSpeed_0                     = 0x0670,
    DeviceMacAddressHigh_1          = 0x0680,
    DeviceMacAddressLow_1           = 0x0684,
    NetworkInterfaceCapability_1    = 0x0688,
    NetworkInterfaceConfiguration_1 = 0x068C,
    CurrentIPAddress_1              = 0x069C,
    CurrentSubnetMask_1             = 0x06AC,
    CurrentDefaultGateway_1         = 0x06BC,
    PersistentIPAddress_1           = 0x06CC,
    PersistentSubnetMask_1          = 0x06DC,
    PersistentDefaultGateway_1      = 0x06EC,
    LinkSpeed_1                     = 0x06F0,
    DeviceMacAddressHigh_2          = 0x0700,
    DeviceMacAddressLow_2           = 0x0704,
    NetworkInterfaceCapability_2    = 0x0708,
    NetworkInterfaceConfiguration_2 = 0x070C,
    CurrentIPAddress_2              = 0x071C,
    CurrentSubnetMask_2             = 0x072C,
    CurrentDefaultGateway_2         = 0x073C,
    PersistentIPAddress_2           = 0x074C,
    PersistentSubnetMask_2          = 0x075C,
    PersistentDefaultGateway_2      = 0x076C,
    LinkSpeed_2                     = 0x0770,
    DeviceMacAddressHigh_3          = 0x0780,
    DeviceMacAddressLow_3           = 0x0784,
    NetworkInterfaceCapability_3    = 0x0788,
    NetworkInterfaceConfiguration_3 = 0x078C,
    CurrentIPAddress_3              = 0x079C,
    CurrentSubnetMask_3             = 0x07AC,
    CurrentDefaultGateway_3         = 0x07BC,
    PersistentIPAddress_3           = 0x07CC,
    PersistentSubnetMask_3          = 0x07DC,
    PersistentDefaultGateway_3      = 0x07EC,
    LinkSpeed_3                     = 0x07F0,
    NumberOfMessageChannels         = 0x0900,
    NumberOfStreamChannel           = 0x0904,
    NumberOfActionSignals           = 0x0908,
    ActionDeviceKey                 = 0x090C,
    NumberOfActiveLinks             = 0x0910,
    GVSPCapability                  = 0x092C,
    MessageChannelCapability        = 0x0930,
    GVCPCapability                  = 0x0934,
    HeartbeatTimeout                = 0x0938,
    TimestampTickFrequencyHigh      = 0x093C,
    TimestampTickFrequencyLow       = 0x0940,
    TimestampControl                = 0x0944,
    TimestampValueHigh              = 0x0948,
    TimestampValueLow               = 0x094C,
    DiscoveryACKDelay               = 0x0950,
    GVCPConfiguration               = 0x0954,
    PendingTimeout                  = 0x0958,
    ControlSwitchoverKey            = 0x095C,
    GVSPConfiguration               = 0x0960,
    PhysicalLinkConfigurationCapability = 0x0964,
    PhysicalLinkConfiguration       = 0x0968,
    IEEE1588Status                  = 0x096C,
    SheduledActionCommandQueueSize  = 0x0970,
    ControlChannelPrivilege         = 0x0A00,
    PrimaryApplicationPort          = 0x0A04,
    PrimaryApplicationIPAddress     = 0x0A14,
    MessageChannelPort              = 0x0B00,
    MessageChannelDestinationAddress = 0x0B10,
    MessageChannelTransmissionTimeout = 0x0B14,
    MessageChannelRetryCount        = 0x0B18,
    MessageChannelSourcePort        = 0x0B1C,

    // chaque canal stream possède ce schéma et prend 64 octets (0x40)
    StreamChannelPort               = 0x0D00,
    StreamChannelPacketSize         = 0x0D04,
    StreamChannelPacketDelay        = 0x0D08,
    StreamChannelDestinationAddress = 0x0D18,
    StreamChannelSourcePort         = 0x0D1C,
    StreamChannelCapability         = 0x0D20,
    StreamChannelConfiguration      = 0x0D24,
    StreamChannelZone               = 0x0D28,
    StreamChannelZoneDirection      = 0x0D2C,
    ManifestTable                   = 0x9000,

    // chaque action group possède ce schéma et prend 16 octets (0x10)
    ActionGroupKey                  = 0x9800,
    ActionGroupMask                 = 0x9814,

    SpecificManufacturerRegister    = 0xA000
};

enum class BootstrapBlockSize: uint16_t {
    ManufacturerName    = 32,
    ModelName           = 32,
    DeviceVersion       = 32,
    ManufacturerInfo    = 48,
    SerialNumber        = 16,
    UserDefinedName     = 16,
    FirstURL            = 512,
    SecondURL           = 512,
    ManifestTable       = 512
};

enum class DeviceClass: uint8_t {
    TRANSMITTER     = 0,
    RECEIVER        = 1,
    TRANSCEIVER     = 2,
    PERIPHERAL      = 3
};

enum class Endianness {
    LITTLE  = 0,
    BIG     = 1
};

enum class CharacterSetIndex: uint8_t {
    UTF8 = 1
};

//namespace GVCP_Capability_Register
//{
//    constexpr unsigned UN = 0;  // user-defined name
//    constexpr unsigned SN = 1;  // serial number support
//    constexpr unsigned HD = 2;  // heartbeat can be disable
//    constexpr unsigned LS = 3;  // link speed register support
//    constexpr unsigned CAP = 4; // CCP application and IP register support
//    constexpr unsigned MT = 5;  // manifest table support
//    constexpr unsigned TD = 6;  // test data is filled with data from LSFR
//    constexpr unsigned DD = 7;  // discovery ack delay support
//    constexpr unsigned WD = 8;  // discovery ack delay writable
//    constexpr unsigned ES = 9;  // extented status code support
//    constexpr unsigned PAS = 10;// primary swithover support
//    constexpr unsigned A = 25;  // action are supported
//    constexpr unsigned P = 26;  // pending ack support
//    constexpr unsigned ED = 27; // event data support
//    constexpr unsigned E = 28;  // event support
//    constexpr unsigned PR = 29; // packetresend support
//    constexpr unsigned W = 30;  // writemem support
//    constexpr unsigned C = 31;  // multipple operation in a single message support
//}

enum class CCPPrivilege {
    OpenAcces,
    ControlAcces,
    ControlAccesWithSwitchoverEnabled,
    ExclusiveAcces,
    AccesDenied
};

struct CCP
{
    static CCPPrivilege privilegeFromCCP(uint32_t CCP);
    static uint32_t CCPfromPrivilege(CCPPrivilege privilege);
    static std::string privilegeToString(CCPPrivilege priv);
};






//class BootstrapRegistersPrivate;
class BootstrapRegisters
{

public:


    static uint32_t streamChannelTranspose(uint32_t channel, BootstrapAddress address) noexcept;

    //    BootstrapRegisters();
    //    ~BootstrapRegisters();
    //    void setNetworkInterface(const QNetworkInterface &netInterface);

    //    uint32_t xmlAddress() const;
    //    void swapXmlFile(constexpr const char * &newFile);

    //    /*!
    //     * \brief addressIsValid
    //     * Contrôle que l'adresse est une adresse de base de registre.
    //     * \param address l'adresse à tester.
    //     * \return true si l'adresse est une adresse de base, false dans le cas contraire.
    //     */
    //    bool addressIsValid(uint32_t address) const;

    //    /*!
    //     * \brief length
    //     * Obtient le nombre d'octet contenu par l'adresse de base.
    //     * \param address L'adresse de base du registre
    //     * \return le nombre d'octets contenus à cette adresse.
    //     */
    //    uint length(uint32_t address) const;

    //    void extendRegisters(uint32_t address, uint32_t value);

    //    uint32_t value(uint32_t address) const;
    //    uint16_t setValue(uint32_t address, uint32_t value);
    //    ReadmemResult readmemPtr(uint32_t address, uint length) const;
    //    WritememResult writememPtr(uint32_t address, uint length) const;

    //    const char *constBlock(uint32_t address) const;
    static uint32_t align(uint32_t val);
    //private:
    //    const QScopedPointer<BootstrapRegistersPrivate> d;

};

} // namespace Gvcp

} // namespace Jgv

#endif // BOOTSTRAPREGISTERS_H
