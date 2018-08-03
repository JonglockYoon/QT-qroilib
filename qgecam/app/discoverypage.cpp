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

#include "discoverypage.h"
#include "discoverypage_p.h"
#include "lineseparator.h"
#include "gvcputils.h"
#include "discoveryhelper.h"
#include "bootstrapregisters.h"

#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QNetworkAddressEntry>

using namespace Qroilib;

DiscoveryPage::DiscoveryPage(QSharedPointer<GvcpUtils> discoverer, QWidget *parent)
    : JgvWizardPage(discoverer, *new DiscoveryPagePrivate, parent)
{
    Q_D(DiscoveryPage);
    setTitle(trUtf8("DISCOVERY"));
    setSubTitle (trUtf8 ("DISCOVERY allows automatic detection of all devices"
                            "GiGE complying with the <b> Device discovery </ b>"));

    d->controllerIp = new QLabel;
    d->controllerIp->setAlignment(Qt::AlignCenter);
    d->controllerIp->setStyleSheet("background-color: white");
    d->transmitterInfos = new QLabel;
    d->transmitterInfos->setStyleSheet("background-color: white");
    d->discover = new QPushButton(trUtf8("Start"));
    d->allNetworks = new QCheckBox(trUtf8("All networks"));

    QLabel *infos = new QLabel(trUtf8("For asymmetric routing : \n # echo 2 > /proc/sys/net/ipv4/conf/all/rp_filter"));
    infos->setVisible(false);
    connect(d->allNetworks, &QCheckBox::toggled, infos, &QLabel::setVisible);

    //    QRegExp rx( "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}" );


    d->devicesList = new QComboBox;
    d->devicesList->setEnabled(false);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->addRow(trUtf8("Controller"), d->controllerIp);
    formLayout->addRow(new LineSeparator);
    formLayout->addRow(trUtf8("Transmitter (camera)"), d->transmitterInfos);
    formLayout->addRow(new LineSeparator);
    QHBoxLayout *dl = new QHBoxLayout;
    dl->addWidget(d->discover,1);
    dl->addWidget(d->allNetworks);
    formLayout->addRow(trUtf8("List of discoveries"), d->devicesList);
    formLayout->addRow(trUtf8("Discover"), dl);
    formLayout->addWidget(infos);


    setLayout(formLayout);

    connect(d->discover, &QPushButton::clicked, [d] () {
        d->devicesList->setEnabled(false);
        d->devicesList->clear();
        d->gvcpUtils->discover(d->allNetworks->isChecked()? QHostAddress::Broadcast: d->gvcpUtils->controller().broadcast());
    });
    connect(d->devicesList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [d] (int index) {
        if (index >= 0) {
            const Jgv::Gvcp::DISCOVERY_ACK ack = d->devicesList->itemData(index).value<Jgv::Gvcp::DISCOVERY_ACK>();
            d->transmitterInfos->setText(ackTohtml(ack));
        }
    });
    connect(d->gvcpUtils.data(), &GvcpUtils::newDeviceDiscovered, [d](const Jgv::Gvcp::DISCOVERY_ACK &ack) {
        const quint32 deviceMode = Jgv::Gvcp::DiscoveryAckHelper::deviceMode(ack);
        if (Jgv::Gvcp::DiscoveryAckHelper::deviceClass(deviceMode) == Jgv::Gvcp::enumType(Jgv::Gvcp::DeviceClass::TRANSMITTER)) {
            QVariant variant;
            variant.setValue(ack);
            d->devicesList->addItem(QByteArray(Jgv::Gvcp::DiscoveryAckHelper::modelName(ack), static_cast<uint16_t>(Jgv::Gvcp::BootstrapBlockSize::ModelName)), variant);
            d->devicesList->setEnabled(true);
        }
    });
}

DiscoveryPage::~DiscoveryPage()
{}

void DiscoveryPage::initializePage()
{
    Q_D(DiscoveryPage);
    d->controllerIp->setText(QString("%0 / %1")
                             .arg(d->gvcpUtils->controller().ip().toString())
                             .arg(d->gvcpUtils->controller().netmask().toString()));
    d->devicesList->clear();
    Jgv::Gvcp::DISCOVERY_ACK transmitter = {};
    d->gvcpUtils->setTransmitterAck(transmitter);
    d->transmitterInfos->setText(ackTohtml(transmitter));
    d->allNetworks->setChecked(false);
    d->gvcpUtils->listenForBroadcast();
}

bool DiscoveryPage::validatePage()
{
    Q_D(DiscoveryPage);
    const Jgv::Gvcp::DISCOVERY_ACK ack = d->devicesList->currentData().value<Jgv::Gvcp::DISCOVERY_ACK>();
    d->gvcpUtils->setTransmitterAck(ack);
    return (d->devicesList->count() > 0);
}

void DiscoveryPage::cleanupPage()
{
    Q_D(DiscoveryPage);
    d->gvcpUtils->stopBroadcastListener();
}




