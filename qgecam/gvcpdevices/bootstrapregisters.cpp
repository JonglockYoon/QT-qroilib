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

#include "bootstrapregisters.h"
#include "bootstrapregisters_p.h"

using namespace Jgv::Gvcp;

//#include "gvcp.h"
//#include "endiantools.h"


//#include <QDomDocument>
//#include <QMap>
//#include <QByteArray>
//#include <QString>
//#include <QStringList>
//#include <QFile>
//#include <QNetworkInterface>
//#include <QPair>
//#include <cstring>

//static bool isAligned(uint val) {return val%4 == 0;}



//static void addProperty(QDomDocument &dom, QDomElement &parent, constexpr const char * &tag, constexpr const char * &value) {
//    QDomElement child = dom.createElement(tag);
//    child.appendChild(dom.createTextNode(value));
//    parent.appendChild(child);
//}

//Reg::Reg(uint32_t val, bool readOnly)
//    : data(new uint8_t[sizeof(val)]), length(sizeof(val)), isReadOnly(readOnly)
//{
//    toBigEndian(val, data);
//}

//Reg::Reg(int maxLength, constexpr const char * &val, bool readOnly)
//    : data(new uint8_t[maxLength]), length(maxLength), isReadOnly(readOnly)
//{
//    Q_ASSERT(length >= 4);
//    std::fill(data, data + maxLength, 0);
//    std::memcpy(data, val.toUtf8().data(), qMin(val.length(), maxLength));
//}

//Reg::~Reg()
//{
//    delete[] data;
//}

//uint32_t Reg::value() const
//{
//    return fromBigEndian<uint32_t>(data);
//}

//void Reg::setValue(uint32_t value)
//{
//    toBigEndian<uint32_t>(value, data);
//}

//BootstrapRegistersPrivate::BootstrapRegistersPrivate()
//{
//    // DOM XML
//    QDomDocument dom;
//    QDomProcessingInstruction prolog = dom.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
//    dom.appendChild(prolog);
//    QDomElement root = dom.createElement("RegisterDescription");
//    root.setAttribute("ModelName", "QroilibReceiver");
//    root.setAttribute("VendorName", "BALETAUD");
//    root.setAttribute("MajorVersion", "1");
//    root.setAttribute("MinorVersion", "0");
//    root.setAttribute("SubMinorVersion", "0");
//    root.setAttribute("ProductGuid", "00000000-0000-0000-0000-000000000000");
//    root.setAttribute("VersionGuid", "00000000-0000-0000-0000-000000000000");
//    root.setAttribute("StandardNameSpace", "None");
//    root.setAttribute("SchemaMajorVersion", "1");
//    root.setAttribute("SchemaMinorVersion", "0");
//    root.setAttribute("SchemaSubMinorVersion", "1");
//    root.setAttribute("xmlns", "http://www.genicam.org/GenApi/Version_1_0");
//    root.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
//    root.setAttribute("xsi:schemaLocation", "http://www.genicam.org/GenApi/Version_1_0 http://www.genicam.org/GenApi/GenApiSchema_Version_1_0.xsd");
//    dom.appendChild(root);


//    QDomElement element = dom.createElement("Category");
//    element.setAttribute("Name", "Root");
//    QDomElement child = dom.createElement("pFeature");
//    child.appendChild(dom.createTextNode("DeviceControl"));
//    element.appendChild(child);
//    root.appendChild(element);

//    element = dom.createElement("Category");
//    element.setAttribute("Name", "DeviceControl");
//    addProperty(dom, element, "DisplayName", "Device control");
//    addProperty(dom, element, "pFeature", "GevCurrentIPAddress");
//    addProperty(dom, element, "pFeature", "DeviceVendorName");
//    addProperty(dom, element, "pFeature", "ModelName");
//    addProperty(dom, element, "pFeature", "ManufacturerInfo");
//    addProperty(dom, element, "pFeature", "UserDefinedName");
//    root.appendChild(element);

//    element = dom.createElement("Integer");
//    element.setAttribute("Name", "GevCurrentIPAddress");
//    addProperty(dom, element, "DisplayName", "GEV Current IP Address");
//    addProperty(dom, element, "pValue", "CurrentIpAddress");
//    addProperty(dom, element, "Inc", "1");
//    addProperty(dom, element, "Representation", "HexNumber");
//    root.appendChild(element);

//    element = dom.createElement("IntReg");
//    element.setAttribute("Name", "CurrentIpAddress");
//    addProperty(dom, element, "Address", "0x0024");
//    addProperty(dom, element, "Length", "4");
//    addProperty(dom, element, "AccessMode", "RO");
//    addProperty(dom, element, "pPort", "Device");
//    addProperty(dom, element, "Sign", "Unsigned");
//    addProperty(dom, element, "Endianess", "BigEndian");
//    root.appendChild(element);

//    element = dom.createElement("StringReg");
//    element.setAttribute("Name", "DeviceVendorName");
//    addProperty(dom, element, "Address", "0x48");
//    addProperty(dom, element, "Length", "32");
//    addProperty(dom, element, "AccessMode", "RO");
//    addProperty(dom, element, "pPort", "Device");
//    root.appendChild(element);

//    element = dom.createElement("StringReg");
//    element.setAttribute("Name", "ModelName");
//    addProperty(dom, element, "Address", "0x68");
//    addProperty(dom, element, "Length", "32");
//    addProperty(dom, element, "AccessMode", "RO");
//    addProperty(dom, element, "pPort", "Device");
//    root.appendChild(element);

//    element = dom.createElement("StringReg");
//    element.setAttribute("Name", "ManufacturerInfo");
//    addProperty(dom, element, "Address", "0x00a8");
//    addProperty(dom, element, "Length", "48");
//    addProperty(dom, element, "AccessMode", "RO");
//    addProperty(dom, element, "pPort", "Device");
//    root.appendChild(element);

//    element = dom.createElement("StringReg");
//    element.setAttribute("Name", "UserDefinedName");
//    addProperty(dom, element, "Address", "0x00e8");
//    addProperty(dom, element, "Length", "16");
//    addProperty(dom, element, "AccessMode", "RW");
//    addProperty(dom, element, "pPort", "Device");
//    root.appendChild(element);




//    element = dom.createElement("Port");
//    element.setAttribute("Name", "Device");
//    root.appendChild(element);

//    QFile kk("/home/cyril/kk.xml");
//    kk.open(QIODevice::WriteOnly);
//    QTextStream out(&kk);
//    dom.save(out, 2);
//    kk.close();

//    xmlAddress = 0xB000;
//    QString xml = dom.toString(2);
//    insert(xmlAddress, new Reg(BootstrapRegisters::align(xml.count()), xml));

//    // BOOTSTRAP
//    insert(Register::Version, new Reg(0x00010002));
//    insert(Register::DeviceMode, new Reg(0x00000000 | (Endianness::BIG<<31) | (DeviceClass::RECEIVER<<28) | CharacterSetIndex::UTF8));
//    insert(Register::DeviceMacAddressHigh_0, new Reg(0x00u));
//    insert(Register::DeviceMacAddressLow_0, new Reg(0x00u));
//    insert(Register::SupportedIPconfiguration_0, new Reg(0x00000006u));
//    insert(Register::CurrentIPconfiguration_0, new Reg(0x00000006u));
//    insert(Register::CurrentIPAddress_0, new Reg(0x00u));
//    insert(Register::CurrentSubnetMask_0, new Reg(0x00u));
//    insert(Register::CurrentDefaultGateway_0, new Reg(0x00u));
//    insert(Register::NumberOfNetworkInterfaces, new Reg(0x00000001u));
//    insert(Register::NumberOfMessageChannels, new Reg(0x00000000u));
//    insert(Register::NumberOfStreamChannel, new Reg(0x00000001u));
//    insert(Register::GVCPCapability, new Reg(0b00000000000000000000000000000011u));
//    insert(Register::HeartbeatTimeout, new Reg(3000u, false));
//    insert(Register::ControlChannelPrivilege, new Reg(0x00000000u, false));
//    insert(Register::StreamChannelPort, new Reg(0x00000000u, false));
//    insert(Register::StreamChannelPacketSize, new Reg(1500u));
//    insert(Register::StreamChannelDestinationAddress, new Reg(0x00u, false));


//    insert(MemoryBlocs::ManufacturerName, new Reg(MemoryBlocs::ManufacturerNameSize, QObject::trUtf8("C.BALETAUD")));
//    insert(MemoryBlocs::ModelName, new Reg(MemoryBlocs::ModelNameSize, QObject::trUtf8("Qroilib Receiver")));
//    insert(MemoryBlocs::DeviceVersion, new Reg(MemoryBlocs::DeviceVersionSize, QObject::trUtf8("1.0 Beta")));
//    insert(MemoryBlocs::SerialNumber, new Reg(MemoryBlocs::SerialNumberSize, QObject::trUtf8("1")));
//    insert(MemoryBlocs::ManufacturerInfo, new Reg(MemoryBlocs::ManufacturerInfoSize, QObject::trUtf8("For testing only")));
//    insert(MemoryBlocs::FirstURL, new Reg(MemoryBlocs::FirstURLSize, QObject::trUtf8("Local:Qroilib_receiver.xml;%0;%1").arg(xmlAddress,0,16).arg(xml.count(),0,16)));
//    insert(MemoryBlocs::SecondURL, new Reg(MemoryBlocs::SecondURLSize, QObject::trUtf8("")));
//    insert(MemoryBlocs::UserDefinedName, new Reg(MemoryBlocs::UserDefinedNameSize, QObject::trUtf8(""), false));
//}

//BootstrapRegistersPrivate::~BootstrapRegistersPrivate()
//{
//    qDeleteAll(*this);
//}

//BootstrapRegisters::BootstrapRegisters()
//    : d(new BootstrapRegistersPrivate)
//{}

//BootstrapRegisters::~BootstrapRegisters()
//{}

//void BootstrapRegisters::setNetworkInterface(const QNetworkInterface &netInterface)
//{
//    bool ok = false;
//    quint64 hw = netInterface.hardwareAddress().remove(":").toULongLong(&ok, 16);
//    if (ok) {
//        d->insert(Register::DeviceMacAddressHigh_0, new Reg(hw >> 32));
//        d->insert(Register::DeviceMacAddressLow_0, new Reg(hw & 0xFFFFFFFF));
//    }

//    QList<QNetworkAddressEntry> a = netInterface.addressEntries();
//    if (!netInterface.addressEntries().isEmpty()) {
//        d->insert(Register::CurrentIPAddress_0, new Reg(a.at(0).ip().toIPv4Address()));
//        d->insert(Register::CurrentSubnetMask_0, new Reg(a.at(0).netmask().toIPv4Address()));
//    }

//    QFile route("/proc/net/route");
//    if (route.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        QByteArray bloc = route.readAll();
//        QTextStream in(&bloc);
//        while (!in.atEnd()) {
//            QString line = in.readLine().simplified();
//            if (line.startsWith(netInterface.humanReadableName())) {
//                QStringList fields = line.split(" ");
//                if ((fields.count() >= 2) && fields.at(1).compare("00000000")==0) {
//                    bool ok = false;
//                    uint32_t val = fields.at(2).toUInt(&ok,16);
//                    if (ok) {
//                        d->insert(Register::CurrentDefaultGateway_0, new Reg(boost::endian::big_to_native(val)));
//                        break;
//                    }
//                }
//            }
//        }
//        route.close();
//    }

//}

//uint32_t BootstrapRegisters::xmlAddress() const
//{
//    return d->xmlAddress;
//}

//void BootstrapRegisters::swapXmlFile(constexpr const char * &newFile)
//{
//    Reg *reg = d->take(d->xmlAddress);
//    delete reg;
//    d->insert(d->xmlAddress, new Reg(align(newFile.count()), newFile));
//    reg = d->take(MemoryBlocs::FirstURL);
//    delete reg;
//    d->insert(MemoryBlocs::FirstURL, new Reg(MemoryBlocs::FirstURLSize, QObject::trUtf8("Local:Qroilib_receive2r.xml;%0;%1").arg(d->xmlAddress,0,16).arg(newFile.count(),0,16)));


//}

//bool BootstrapRegisters::addressIsValid(uint32_t address) const
//{
//    return d->contains(address);
//}

//uint BootstrapRegisters::length(uint32_t address) const
//{
//    Q_ASSERT(d->contains(address));
//    return d->value(address)->length;
//}

//void BootstrapRegisters::extendRegisters(uint32_t address, uint32_t value)
//{
//    d->insert(address, new Reg(value, false));
//}

//uint32_t BootstrapRegisters::value(uint32_t address) const
//{
//    Q_ASSERT(d->contains(address));
//    return (d->value(address)->value());
//}

//uint16_t BootstrapRegisters::setValue(uint32_t address, uint32_t value)
//{
//    Q_ASSERT(d->contains(address));
//    Reg *reg = d->value(address);

//    if (reg->isReadOnly) return STATUS::WRITE_PROTECT;

//    d->value(address)->setValue(value);
//    return STATUS::SUCCESS;
//}

//ReadmemResult BootstrapRegisters::readmemPtr(uint32_t address, uint length) const
//{
//    // contrôle l'alignement
//    if (!isAligned(address) || !isAligned(length)) {
//        return ReadmemResult(STATUS::BAD_ALIGNMENT, 0);
//    }

//    // obtient l'addresse du bloc suceptible
//    // de porter l'information
//    uint32_t base = (d->contains(address)) ? address : (--d->upperBound(address)).key();
//    uint32_t offset = address - base;
//    const Reg *reg = d->value(base);

//    // si pas assez de place
//    if ((offset+length) > reg->length) {
//        return ReadmemResult(STATUS::INVALID_PARAMETER, 0);
//    }

//    return ReadmemResult(STATUS::SUCCESS, reg->data + offset);
//}


//WritememResult BootstrapRegisters::writememPtr(uint32_t address, uint length) const
//{
//    // contrôle l'alignement
//    if (!isAligned(address) || !isAligned(length)) {
//        return WritememResult(STATUS::BAD_ALIGNMENT, 0);
//    }

//    // obtient l'addresse du bloc suceptible
//    // de porter l'information
//    uint32_t base = (d->contains(address)) ? address : (--d->upperBound(address)).key();
//    uint32_t offset = address - base;
//    const Reg *reg = d->value(base);


//    if (reg->isReadOnly) {
//        return WritememResult(STATUS::WRITE_PROTECT, 0);
//    }

//    // si pas assez de place
//    if ((offset+length) > reg->length) {
//        return WritememResult(STATUS::INVALID_PARAMETER, 0);
//    }

//    return WritememResult(STATUS::SUCCESS, reg->data + offset);

//}

//const char *BootstrapRegisters::constBlock(uint32_t address) const
//{
//    Q_ASSERT(d->contains(address));
//    return reinterpret_cast<const char *>(d->value(address)->data);
//}

uint32_t BootstrapRegisters::streamChannelTranspose(uint32_t channel, BootstrapAddress address) noexcept
{
    return (channel * 0x40) + enumType(address);
}

uint32_t BootstrapRegisters::align(uint32_t val)
{
    const int r = val % 4;
    return r == 0 ? val : val + (4 - r);
}


CCPPrivilege CCP::privilegeFromCCP(uint32_t CCP)
{
    if ((CCP&0x01)!=0) return CCPPrivilege::ExclusiveAcces;
    if ((CCP&0x02)==0) return CCPPrivilege::OpenAcces;
    if ((CCP&0x04)==0) return CCPPrivilege::ControlAcces;
    return CCPPrivilege::ControlAccesWithSwitchoverEnabled;
}

uint32_t CCP::CCPfromPrivilege(CCPPrivilege privilege) {
    if (privilege==CCPPrivilege::OpenAcces) return 0x00;
    if (privilege==CCPPrivilege::ControlAcces) return 0x02;
    if (privilege==CCPPrivilege::ControlAccesWithSwitchoverEnabled) return 0x06;
    return 0x01;
}

std::string CCP::privilegeToString(CCPPrivilege priv) {
    if (priv == CCPPrivilege::OpenAcces) return "OpenAcces";
    if (priv == CCPPrivilege::ControlAcces) return "ControlAcces";
    if (priv == CCPPrivilege::ControlAccesWithSwitchoverEnabled) return "ControlAccesWithSwitchoverEnabled";
    if (priv == CCPPrivilege::ExclusiveAcces) return "ExclusiveAcces";

    return "AccesDenied";
}

//int bitFromRegister(uint32_t reg, unsigned bit) {
//    return 0x00000001 & (reg >> (31-bit));
//}
