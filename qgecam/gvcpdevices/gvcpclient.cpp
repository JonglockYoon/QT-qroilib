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

#include "gvcpclient.h"
#include "gvcpclient_p.h"

#include "headerhelper.h"
#include "discoveryhelper.h"
#include "readreghelper.h"
#include "writereghelper.h"
#include "readmemhelper.h"
#include "writememhelper.h"
#include "bootstrapregisters.h"
#include "gvcp.h"
#include "timestampdate.h"

#include <time.h>

#include <QDebug>
#include <QDateTime>
#include <QtWidgets>
#include <QtNetwork>
#include <iostream>

#ifndef Q_OS_WIN
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <poll.h>
#include <time.h>
#endif

#define D(Class) Class##Private * const d = d_func()

using namespace Jgv::Gvcp;

namespace {
union Buffer {
    char data[GVCP_PACKET_MAX_SIZE];
    CMD_HEADER headerCmd;
    ACK_HEADER headerAck;
    DISCOVERY_ACK discoveryAck;
    READREG_CMD readregCmd;
    READREG_ACK readregAck;
    WRITEREG_CMD writeregCmd;
    WRITEREG_ACK writeregAck;
    READMEM_CMD readmemCmd;
    READMEM_ACK readmemAck;
    WRITEMEM_CMD writememCmd;
};
Buffer buffer = {};

//#define enumType(x) x

std::string statusToString(uint16_t status) {
    switch (status) {
    case enumType(Status::SUCCESS): return "Succes";
    case enumType(Status::PACKET_RESEND): return "Packet resend";
    case enumType(Status::NOT_IMPLEMENTED): return "Not implemented";
    case enumType(Status::INVALID_PARAMETER): return "Invalid parameter";
    case enumType(Status::INVALID_ADDRESS): return "Invalid address";
    case enumType(Status::WRITE_PROTECT): return "Write protect";
    case enumType(Status::BAD_ALIGNMENT): return "Bad alignement";
    case enumType(Status::ACCESS_DENIED): return "Acces denied";
    case enumType(Status::PACKET_UNAVAILABLE): return "Packet unavailable";
    case enumType(Status::DATA_OVERUN): return "Data overun";
    case enumType(Status::INVALID_HEADER): return "Invalid header";
    case enumType(Status::PACKET_NOT_YET_AVAILABLE): return "Packet not yet available";
    case enumType(Status::PACKET_AND_PREV_REMOVED_FROM_MEMORY): return "Packet and prev removed from memory";
    case enumType(Status::PACKET_REMOVED_FROM_MEMORY): return "Packet removed from memory";
    case enumType(Status::NO_REF_TIME): return "No ref time";
    case enumType(Status::PACKET_TEMPORARILY_UNAVAILABLE): return "packet temporarily unavailable";
    case enumType(Status::STATUS_OVERFLOW): return "Status overflow";
    case enumType(Status::ACTION_LATE): return "Action late";
    case enumType(Status::SERROR): return "Error";
    default: return "Unknow error";
    }
}

} // anonymous namespace


#ifdef Q_OS_WIN

bool ClientPrivate::proceed(HeaderCmdHelper &cmd, uint16_t ackType, bool resetHeartbeat)
{
    // Ensures the threadsafe call
    std::unique_lock<std::mutex> lock(proceedMutex);

    // relance le timer heartbeat
    if (resetHeartbeat) {
        cvHB.notify_all();
    }

    // For reading standby
    const uint16_t currentId = id;
    id = (id == 0xFFFF) ? 1 : id + 1;
    cmd.setReqId(currentId);

    sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(GVCP_PORT);
    dest.sin_addr.s_addr = htonl(deviceIP);
    socklen_t destlen = sizeof(dest);


//    struct timeval read_timeout;
//    read_timeout.tv_sec = 0;
//    read_timeout.tv_usec = 200;
//    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&read_timeout, sizeof read_timeout);

    int retry = cmdRetry;

    QHostAddress qhost(deviceIP);
    qDebug() << "GvcpClientPrivate::proceed sendto " << qhost.toString();


    // As long as the cycle of the retry is not finished, we re-issue the command
    while (retry > 0) {
        --retry;

        // Ships without blocking, to accelerate requests without ACK (RESENDPACKET in particular)
//        QHostAddress host = QHostAddress(deviceIP);
//        int sendlen = sd->writeDatagram(cmd.data, cmd.size, host, GVCP_PORT);
        int sendlen = sendto(sd, cmd.data, cmd.size, 0/*MSG_DONTWAIT*/, reinterpret_cast<const sockaddr *>(&dest), destlen);

        // The shipment is not passed
        if (sendlen == -1) {
            qDebug() << ("GvcpClient send failed");
        }
        if (static_cast<std::size_t>(sendlen) != cmd.size) {
            qDebug() << "GvcpClientPrivate::proceed: Oups! send len != command size";
        }

        // If the command does not need to be acknowledged,
        if (!cmd.acknowledge()) {
            return true;
        }

        QTime time;
        time.start();
        while (true) {
            int nElapsed = time.elapsed();
            if (nElapsed > 1000)
                break;

            sockaddr_in sender = {};
            socklen_t senderlen = sizeof(sender);
            int recvlen = recvfrom(sd, buffer.data, GVCP_PACKET_MAX_SIZE, 0, reinterpret_cast<sockaddr *>(&sender), &senderlen);
            //int recvlen = recv(sd, buffer.data, GVCP_PACKET_MAX_SIZE, 0);
            if (recvlen < 0) {
                // raison inconnue, on se remet en attente poll
                qDebug() << "GvcpClient proceed error";
            }

            time.start();

            // controle minimum
            if (!HeaderAckHelper::isValid(buffer.headerAck, recvlen)) {
                // la taille est incohérente, on se remet en poll
                qDebug() << "GvcpClientPrivate::proceed: RecvLen errror: " << recvlen;// << std::endl;
            }

            // Ensures the source
            else if (ntohl(sender.sin_addr.s_addr) != deviceIP || ntohs(sender.sin_port) != GVCP_PORT) {
                // le paquet ne vient pas du device, on se remet en attente poll
                qDebug() << "GvcpClientPrivate::proceed: invalide peer ! " << inet_ntoa(sender.sin_addr);// << std::endl;
            }

            // the response ID must match the ID of the command
             else if (HeaderAckHelper::ackId(buffer.headerAck) != cmd.reqId()) {
                // the packet is not the response to the command, it may be the response of a previous slow command (timeout but packet not lost)
                // we only report if the ack is older than 1 ID
                if (HeaderAckHelper::ackId(buffer.headerAck) != (cmd.reqId() - 1)) {
                    qDebug() << "GvcpClientPrivate::proceed: Ack id != Cmd id: " << HeaderAckHelper::ackId(buffer.headerAck) << " " << cmd.reqId();// << std::endl;
                }
            }

            // Check that the ack is of the right type
            else if (HeaderAckHelper::acknowledge(buffer.headerAck) != ackType) {
                // Bad ack, we put back on hold poll
                qDebug() << "GvcpClientPrivate::proceed: Bad Ack: " << HeaderAckHelper::acknowledge(buffer.headerAck) << " " << ackType;// << std::endl;
            }

            //The status is not successful
            else if (HeaderAckHelper::status(buffer.headerAck) != enumType(Status::SUCCESS)) {
                // The command has not passed, we leave
                qDebug() << "GvcpClient failed " << QString::fromStdString(statusToString(HeaderAckHelper::status(buffer.headerAck)));
                return false ;
            }

            //All goes well, we go out
            else {
                qDebug() << "ClientPrivate::proceed ok: " << recvlen;
                return true ;
            }

        }
    }


    qDebug() << "GvcpClientPrivate::proceed: complete cycle failed";// << std::endl;

    return false;

}
#else

bool ClientPrivate::proceed(HeaderCmdHelper &cmd, uint16_t ackType, bool resetHeartbeat)
{
    // assure l'appel threadsafe
    std::unique_lock<std::mutex> lock(proceedMutex);

    // relance le timer heartbeat
    if (resetHeartbeat) {
        cvHB.notify_all();
    }

    // pour l'attente de lecture
    pollfd pfd = {};
    pfd.fd = sd;
    pfd.events = POLLIN;

    const uint16_t currentId = id;
    id = (id == 0xFFFF) ? 1 : id + 1;
    cmd.setReqId(currentId);

    sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(GVCP_PORT);
    dest.sin_addr.s_addr = htonl(deviceIP);
    socklen_t destlen = sizeof(dest);

    int retry = cmdRetry;

    // tant que le cycle des retry n'est pas fini, on réemet la commande
    while (retry > 0) {
        --retry;

        // expédie sans bloquer, pour accélérer les demandes sans ACK (RESENDPACKET en particulier)
        ssize_t sendlen = sendto(sd, cmd.data, cmd.size, 0/*MSG_DONTWAIT*/, reinterpret_cast<const sockaddr *>(&dest), destlen);

        // l'envoi n'est pas passé
        if (sendlen == -1) {
            std::perror("GvcpClient failed");
        }
        if (static_cast<std::size_t>(sendlen) != cmd.size) {
            std::cerr << "GvcpClientPrivate::proceed: Oups! send len != command size" << std::endl;
        }

        // si la commande n'a pas besoin d'acquiescement, on quitte
        if (!cmd.acknowledge()) {
            return true;
        }


        while (1) {
            // se met en attente d'un paquet (200 ms timeout)
            const int pollResult = poll(&pfd, 1, 200);
            if (pollResult == -1) {
                std::perror("GvcpClient proceed");
            }
            else if (pollResult == 0) {
                // aucune réponse dans le temps imparti, on relance un cycle complet
                std::clog << "GvcpClien proceed: poll Timeout!" << std::endl;
                break;
            }
            else {
                sockaddr_in sender = {};
                socklen_t senderlen = sizeof(sender);

                ssize_t recvlen = recvfrom(sd, buffer.data, GVCP_PACKET_MAX_SIZE, 0, reinterpret_cast<sockaddr *>(&sender), &senderlen);
                if (recvlen < 0) {
                    // raison inconnue, on se remet en attente poll
                    std::perror("GvcpClient proceed");
                }

                // controle minimum
                else if (!HeaderAckHelper::isValid(buffer.headerAck, recvlen)) {
                    // la taille est incohérente, on se remet en poll
                    std::cerr << "GvcpClientPrivate::proceed: RecvLen errror: " << recvlen << std::endl;
                }

                // s'assure de la source
                else if ((ntohl(sender.sin_addr.s_addr) != deviceIP) || (ntohs(sender.sin_port) != GVCP_PORT)) {
                    // le paquet ne vient pas du device, on se remet en attente poll
                    std::clog << "GvcpClientPrivate::proceed: invalide peer ! " << inet_ntoa(sender.sin_addr) << std::endl;
                }

                // l'ID de la réponse doit correspondre à l'ID de la commande
                else if (HeaderAckHelper::ackId(buffer.headerAck) != cmd.reqId()) {
                    // le paquet n'est pas la réponse à la commande, elle peut-etre la reponse d'une commande lente précédente (timeout mais paquet pas perdu)
                    // on signale seulement si l'ack est plus vieux d'1 ID
                    if (HeaderAckHelper::ackId(buffer.headerAck) != (cmd.reqId() - 1)) {
                        std::clog << "GvcpClientPrivate::proceed: Ack id != Cmd id: " << HeaderAckHelper::ackId(buffer.headerAck) << " " << cmd.reqId() << std::endl;
                    }
                }

                // controle que l'ack est du bon type
                else if (HeaderAckHelper::acknowledge(buffer.headerAck) != ackType) {
                    // mauvais ack, on se remet  en attente poll
                    std::clog << "GvcpClientPrivate::proceed: Bad Ack: " << HeaderAckHelper::acknowledge(buffer.headerAck) << " " << ackType << std::endl;
                }

                // le status n'est pas succes
                else if (HeaderAckHelper::status(buffer.headerAck) != enumType(Status::SUCCESS)) {
                    // la commande n'est pas passée, on quitte
                    std::clog << "GvcpClient failed " << statusToString(HeaderAckHelper::status(buffer.headerAck)) << std::endl;
                    return false;
                }

                // tout va bien, on sort
                else {
                    return true;
                }
            }
        }
    }
    std::clog << "GvcpClientPrivate::proceed: complete cycle failed" << std::endl;
    return false;

}
#endif
void ClientPrivate::doHeartbeat()
{


    ReadregCmdHelper cmd(sizeof(uint32_t));
    cmd.setAddresses({enumType(BootstrapAddress::HeartbeatTimeout)});

    while (heartbeatEnable) {
        std::unique_lock<std::mutex> lock(mutex);
        if (cvHB.wait_for(lock, std::chrono::milliseconds(heartbeatTimeout)) == std::cv_status::timeout) {
            // obtient le valeur du timeout heartbeat
            if (proceed(cmd, GVCP_ACK_READREG)) {
                std::vector<uint32_t>regs = ReadregAckHelper::aswers(buffer.readregAck);
                if (!regs.empty()) {
                    // on diminue le timeout de 400 ms
                    heartbeatTimeout = regs.at(0) - 400;
                }
            }
        }
    }


}

Client::Client()
    : d_ptr(new ClientPrivate)
{}

Client::~Client()
{
    releaseDevice();
}

bool Client::controleDevice(uint32_t controllerIP, uint32_t deviceIP)
{
    D(Client);

    if (d->state == State::controller && controllerIP == d->controllerIP && deviceIP == d->deviceIP) {
        return true;
    }

    if (d->state != State::none) {
        releaseDevice();
    }

    // descripteur du socket UDP
#if 0//ndef Q_OS_WIN
    d->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    d->sd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
#endif
    if(d->sd < 0)
    {
        qDebug() << "GvcpDiscoverer: failed to create socket";
        return false;
    }

    // bind sur port alléatoire
    sockaddr_in localAddress = {};
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(0);   // port aléatoire
    localAddress.sin_addr.s_addr = htonl(controllerIP);
    if (bind(d->sd, reinterpret_cast<const sockaddr *>(&localAddress), sizeof(sockaddr_in) ) == -1) {
        qDebug() << "GvcpClient::connect bind() error";
#ifndef Q_OS_WIN
        close(d->sd);
#else
        closesocket(d->sd);
#endif
        d->sd = -1;
        return false;
    }

    d->controllerIP = controllerIP;
    d->deviceIP = deviceIP;
    d->state = State::controller;

    // obtient CCP, si lecture impossible on a un soucis de communication
    std::vector<uint32_t> regs = readRegisters({enumType(BootstrapAddress::ControlChannelPrivilege)});

    if (regs.empty()) {
        std::clog << "GvcpClient::connect failed: can't read CCP (exclusive access ?)" << std::endl;
        releaseDevice();
        return false;
    }

    CCPPrivilege devicePriv = CCP::privilegeFromCCP(regs.at(0));
    // s'assure qu'on a le droit de se connecter
    if (devicePriv != CCPPrivilege::OpenAcces) {
        std::clog << "GvcpClient failed to control device, not in OpenAcces" << std::endl;
        releaseDevice();
        return false;
    }

    // demande le privilège control
    if (!writeRegisters({ADDRESS_VALUE{enumType(BootstrapAddress::ControlChannelPrivilege), CCP::CCPfromPrivilege(CCPPrivilege::ControlAcces)}})) {
        std::clog << "GvcpClient failed to control device, can write privilege" << std::endl;
        releaseDevice();
        return false;
    }


    //in_addr addr = {};
    //addr.s_addr = htonl(deviceIP);
    QHostAddress qhost(deviceIP);
    qDebug() << "GvcpClient controls " << qhost.toString();


    // obtient le valeur du timeout heartbeat
    regs = readRegisters({enumType(BootstrapAddress::HeartbeatTimeout)});
    if (!regs.empty()) {
        // on diminue le timeout de 400 ms
        d->heartbeatTimeout = regs.at(0) - 400;
        std::clog << "GvcpClient activates heartbeat " << regs.at(0) << " ms, running at " << d->heartbeatTimeout << std::endl;
    }

    // Launches the heartbeat
    d->timerPtr.reset(new std::thread(&ClientPrivate::doHeartbeat, d));
//    pthread_setname_np(d->timerPtr->native_handle(), "GvcpHeartbeatTimer");

    std::string manufacturerName, modelName;
    const uint8_t *p = readMemory(enumType(BootstrapAddress::ManufacturerName), enumType(BootstrapBlockSize::ManufacturerName));
    if (p != nullptr) {
        manufacturerName = std::string(reinterpret_cast<const char *>(p));
    }
    p = readMemory(enumType(BootstrapAddress::ModelName), enumType(BootstrapBlockSize::ModelName));
    if (p != nullptr) {
        modelName = std::string(reinterpret_cast<const char *>(p));
    }

    qDebug() << "GvcpClient device is " << QString::fromStdString(manufacturerName) << " " << QString::fromStdString(modelName );


    return true;
}

bool Client::monitorDevice(uint32_t monitorIP, uint32_t deviceIP)
{
    D(Client);

    if (d->state == State::monitor && monitorIP == d->controllerIP && deviceIP == d->deviceIP) {
        //in_addr addr = {};
        //addr.s_addr = htonl(deviceIP);
        QHostAddress qhost(deviceIP);
        qDebug() << "GvcpClient allready monitoring " << qhost.toString();
        return true;
    }

    if (d->state != State::monitor) {
        releaseDevice();
    }

#ifndef Q_OS_WIN
    d->sd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
#else
    d->sd = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
#endif
    if(d->sd < 0)
    {
        qDebug() << "GvcpClient failed to monitor device";
        return false;
    }

    // bind sur port alléatoire
    sockaddr_in localAddress = {};
    localAddress.sin_family = AF_INET;
    localAddress.sin_port = htons(0);   // port aléatoire
    localAddress.sin_addr.s_addr = htonl(monitorIP);
    if (bind(d->sd, reinterpret_cast<const sockaddr *>(&localAddress), sizeof(sockaddr_in) ) == -1) {
        qDebug() << "GvcpClient failed to monitor device error";
#ifndef Q_OS_WIN
        int rtn = close(d->sd);
#else
        int rtn = closesocket(d->sd);
#endif
        if (rtn == 0) {
            d->sd = -1;
            d->state = State::none;
            d->controllerIP = 0;
            d->deviceIP = 0;
        }
        else {
            qDebug() << "GvcpClient failed to close socket error";
        }
        return false;
    }

    d->state = State::monitor;
    d->controllerIP = monitorIP;
    d->deviceIP = deviceIP;

    // We have a listening socket we are trying to read CCP
    std::vector<uint32_t> regs = readRegisters({enumType(BootstrapAddress::ControlChannelPrivilege)});
    if (regs.empty()) {
        qDebug() << "GvcpClient can't read CCP (exclusive access ?)";
        releaseDevice();
        return false;
    }

    in_addr addr = {};
    addr.s_addr = htonl(deviceIP);
    QHostAddress qhost(deviceIP);
    qDebug() << "GvcpClient monitors " << qhost.toString();


    return true;
}

void Client::releaseDevice()
{
    D(Client);

    if (d->state == State::none) {
        return;
    }

    // If the heartbeat turns, it is closed
    bool b = false;
    if (d->timerPtr)
        b = d->timerPtr->joinable();
    if (b)
    {
        // Cut the heartbeat
        d->heartbeatEnable = false;
        d->cvHB.notify_all();
        d->timerPtr->join();
    }

    // If one is a controller
    if (d->state == State::controller) {
        // We cut the stream
        if (!writeRegisters({ADDRESS_VALUE{enumType(BootstrapAddress::ControlChannelPrivilege), 0x00000000}})) {
            qDebug() << "GvcpClient fails to clear Control Channel Privilege";
        }

    }

    // ferme le socket
#ifndef Q_OS_WIN
        int rtn = close(d->sd);
#else
        int rtn = closesocket(d->sd);
#endif
    if (rtn == 0) {
        in_addr addr = {};
        addr.s_addr = htonl(d->deviceIP);
        char saddr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.s_addr, saddr, INET_ADDRSTRLEN);

        d->sd = -1;
        d->state = State::none;
        d->controllerIP = 0;
        d->deviceIP = 0;

        QHostAddress qhost(d->deviceIP);
        qDebug() << "GvcpClient releases " << qhost.toString();
    }
    else {
        qDebug() << "GvcpClient failed to close socket error";
    }

    d->state = State::none;

}

std::vector<uint32_t> Client::readRegisters(const std::vector<uint32_t> &addresses)
{
    D(Client);

    if (d->state == State::none) {
        std::cerr << "GvcpClient failed to read register, no device configured" << std::endl;
        return std::vector<uint32_t>();
    }

    //Forge control
    ReadregCmdHelper cmd(addresses.size() * sizeof(uint32_t));
    cmd.setAddresses(addresses);

    std::vector<uint32_t> result;
    if (d->proceed(cmd, GVCP_ACK_READREG)) {
        result = ReadregAckHelper::aswers(buffer.readregAck);
    }
    return result;
}

bool Client::writeRegisters(const AddrValPairList &values)
{
    D(Client);
    if (d->state != State::controller) {
        std::cerr << "GvcpClient failed to write register, we are not controller" << std::endl;
        return false;
    }


    //Forge control
    WriteregCmdHelper cmd(values.size() * 2 * sizeof(uint32_t));
    cmd.setRegsValList(values);

    return d->proceed(cmd, GVCP_ACK_WRITEREG);
}

const uint8_t *Client::readMemory(uint32_t address, uint16_t count)
{
    D(Client);

    if (d->state == State::none) {
        std::cerr << "GvcpClient failed to read memory, no device configured" << std::endl;
        return nullptr;
    }

    //Forge control
    ReadmemCmdHelper cmd(READMEM_CMD_LENGTH);
    cmd.setAddress(address);
    cmd.setDataCount(count);

    if (d->proceed(cmd, GVCP_ACK_READMEM)) {
        return ReadmemAckHelper::dataPtr(buffer.readmemAck);
    }
    return nullptr;
}

bool Client::writeMemory(uint32_t address, const uint8_t *data, uint32_t size)
{
    D(Client);

    if (d->state != State::controller) {
        std::cerr << "GvcpClient failed to write memory, we are not controller" << std::endl;
        return false;
    }

    // Forge control
    WritememCmdHelper cmd(sizeof(uint32_t) + size);
    cmd.setAddress(address);
    cmd.setCmdData(data, size);
    return d->proceed(cmd, GVCP_ACK_WRITEMEM);

}

std::string Client::xmlFile()
{
    // on récupère la l'URL
    std::string url {reinterpret_cast<const char *>(readMemory(enumType(BootstrapAddress::FirstURL), enumType(BootstrapBlockSize::FirstURL)))};

    // si ne commence pas par Local: on quitte
    if (url.compare(0, 6, "Local:") != 0) {
        return std::string();
    }

    // cherche les occurences du séparateur de champs
    std::size_t from = 0;
    auto to = url.find_first_of(';');
    std::vector<std::string> split;
    while (to != std::string::npos) {
        split.emplace_back(url.substr(from, to - from));
        from = to + 1;
        to = url.find_first_of(';', to + 1);
    }
    split.emplace_back(url.substr(from));

    // si pas 3 champs, on quitte
    if (split.size() != 3) {
        return std::string();
    }

    std::size_t pos {0};
    auto address = std::stoul(split[1], &pos, 16);
    const auto size = std::stoul(split[2], &pos, 16);
    const auto end = address + size;

    std::clog << url << " " << address << " " << size << std::endl;

    // on construit un tampon de la taille du fichier
    std::vector<char> file (static_cast<std::size_t>(size));
   // std::memcpy(file.data(), readMemory(static_cast<uint32_t>(address), READMEM_ACK_PAYLOAD_MAX_SIZE), READMEM_ACK_PAYLOAD_MAX_SIZE);

    char *dest = file.data();
    while ((address+READMEM_ACK_PAYLOAD_MAX_SIZE) < end) {
        std::memcpy(dest, readMemory(static_cast<uint32_t>(address), READMEM_ACK_PAYLOAD_MAX_SIZE), READMEM_ACK_PAYLOAD_MAX_SIZE);
        address += READMEM_ACK_PAYLOAD_MAX_SIZE;
        dest += READMEM_ACK_PAYLOAD_MAX_SIZE;
    }
    // le dernier segment
    const auto lastSize =  BootstrapRegisters::align(size % READMEM_ACK_PAYLOAD_MAX_SIZE);
    std::memcpy(dest, readMemory(static_cast<uint32_t>(address), static_cast<uint16_t>(lastSize)), static_cast<std::size_t>(lastSize));



    return std::string(file.data());
}

std::string Client::xmlFilename()
{
    // on récupère l'URL
    std::string url {reinterpret_cast<const char *>(readMemory(enumType(BootstrapAddress::FirstURL), enumType(BootstrapBlockSize::FirstURL)))};

    // si ne commence pas par Local: on quitte
    if (url.compare(0, 6, "Local:") != 0) {
        return std::string();
    }

    // cherche les occurences du séparateur de champs
    const auto to = url.find_first_of(';');
    return to!=std::string::npos?url.substr(6, to - 6):std::string();
}

Datation Client::getDatation()
{
    D(Client);

    // Asks the latch of the current timestamp
    WriteregCmdHelper latch(sizeof(ADDRESS_VALUE));
    latch.setRegsValList({ ADDRESS_VALUE {enumType(BootstrapAddress::TimestampControl), 0x00000002} });
    // Reading the 2 registers timestamp
    ReadregCmdHelper readTS(2 * sizeof(uint32_t));
    readTS.setAddresses({ enumType(BootstrapAddress::TimestampValueHigh), enumType(BootstrapAddress::TimestampValueLow) });

    QDateTime start = QDateTime::currentDateTime();
    if (d->proceed(latch, GVCP_ACK_WRITEREG))
    {
        if (d->proceed(readTS, GVCP_ACK_READREG)) {
            std::vector<uint32_t>  result = ReadregAckHelper::aswers(buffer.readregAck);
            uint64_t timestamp = result[0];
            timestamp <<= 32;
            timestamp |= result[1];

            auto toDate = [] (const QDateTime &t) {
                uint64_t msecs = t.toMSecsSinceEpoch();
                return static_cast<uint64_t>(msecs);
            };

            QDateTime stop = QDateTime::currentDateTime();
            return Datation {timestamp , toDate(start), toDate(stop) };
        }
    }

    return Datation {0,0,0};
}


