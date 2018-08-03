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

#include "jgvwizardpage.h"
#include "jgvwizardpage_p.h"

#include "discoveryhelper.h"
#include "camerasession.h"
#include "htmltable.h"

#include <QHostAddress>

using namespace Qroilib;


JgvWizardPage::JgvWizardPage(QSharedPointer<Qroilib::GvcpUtils> discoverer, JgvWizardPagePrivate &dd, QWidget *parent)
    : QWizardPage(parent), d_ptr(&dd)
{
    setMinimumSize(500,400);
    setPixmap(QWizard::WatermarkPixmap, QPixmap("://ressources/icons/Qroilib.png"));
    d_ptr->gvcpUtils = discoverer;
}

JgvWizardPage::~JgvWizardPage()
{}

QString JgvWizardPage::ackTohtml(const Jgv::Gvcp::DISCOVERY_ACK &ack)
{
    const quint16 macH = Jgv::Gvcp::DiscoveryAckHelper::deviceMACaddressHigh(ack);
    const quint32 macL = Jgv::Gvcp::DiscoveryAckHelper::deviceMACaddressLow(ack);
    HtmlTable table;
    table.addRow(trUtf8("model"), Jgv::Gvcp::DiscoveryAckHelper::modelName(ack));
    table.addRow(trUtf8("maker"), Jgv::Gvcp::DiscoveryAckHelper::manufacturerName(ack));
    table.addRow(trUtf8("version"), Jgv::Gvcp::DiscoveryAckHelper::deviceVersion(ack));
    table.addRow(trUtf8("Serial number"), Jgv::Gvcp::DiscoveryAckHelper::serialNumber(ack));
    table.addRow(trUtf8("name"), Jgv::Gvcp::DiscoveryAckHelper::userDefinedName(ack));
    table.addRow(trUtf8("ip"), QHostAddress(Jgv::Gvcp::DiscoveryAckHelper::currentIP(ack)).toString());
    table.addRow(trUtf8("netmask"), QHostAddress(Jgv::Gvcp::DiscoveryAckHelper::currentSubnetMask(ack)).toString());
    table.addRow(trUtf8("gateway"), QHostAddress(Jgv::Gvcp::DiscoveryAckHelper::defaultGateway(ack)).toString());
    table.addRow(trUtf8("mac"), QString("%0:%1:%2:%3:%4:%5")
                 .arg((macH >> 8) & 0xFF,2,16, QLatin1Char('0'))
                 .arg((macH >> 0) & 0xFF,2,16, QLatin1Char('0'))
                 .arg((macL >> 24) & 0xFF,2,16, QLatin1Char('0'))
                 .arg((macL >> 16) & 0xFF,2,16, QLatin1Char('0'))
                 .arg((macL >> 8) & 0xFF,2,16, QLatin1Char('0'))
                 .arg((macL >> 0) & 0xFF,2,16, QLatin1Char('0')).toUpper());
    return table;
}





