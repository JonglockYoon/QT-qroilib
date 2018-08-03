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

#include "forceippage.h"
#include "forceippage_p.h"
#include "gvcputils.h"
#include "discoveryhelper.h"
#include "lineseparator.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QHostAddress>
#include <QNetworkAddressEntry>
#include <QCheckBox>
#include <QRegExpValidator>

using namespace Qroilib;

ForceIPPage::ForceIPPage(QSharedPointer<GvcpUtils> discoverer, QWidget *parent)
    : JgvWizardPage(discoverer, *new ForceIPPagePrivate, parent)
{
    Q_D(ForceIPPage);
    setTitle(trUtf8("FORCEIP"));
    setSubTitle (trUtf8 ("FORCEIP will let you tell the camera what IP"
                            "She must use to communicate with you."));



    d->controllerIp = new QLabel;
    d->controllerIp->setAlignment(Qt::AlignCenter);
    d->controllerIp->setStyleSheet("background-color: white");
    d->transmeterInfos = new QLabel;
    d->transmeterInfos->setStyleSheet("background-color: white");
    QRegExp rx( "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}" );
    d->useForceIP = new QCheckBox;
    d->useForceIP->setChecked(false);
    d->newIP =  new QLineEdit;
    d->newIP->setAlignment(Qt::AlignCenter);
    d->newIP->setValidator(new QRegExpValidator(rx));
    d->newMask =  new QLineEdit;
    d->newMask->setAlignment(Qt::AlignCenter);
    d->newMask->setValidator(new QRegExpValidator(rx));
    d->newGateway =  new QLineEdit;
    d->newGateway->setAlignment(Qt::AlignCenter);
    d->newGateway->setValidator(new QRegExpValidator(rx));
    d->apply = new QPushButton(trUtf8("Apply"));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->addRow(trUtf8("Controller"), d->controllerIp);
    formLayout->addRow(new LineSeparator);
    formLayout->addRow(trUtf8("Transmitter (camera)"), d->transmeterInfos);
    formLayout->addRow(new LineSeparator);
    formLayout->addRow(trUtf8("Using  FORCEIP"), d->useForceIP);
    formLayout->addRow(trUtf8("New IP camera"), d->newIP);
    formLayout->addRow(trUtf8("New camera mask"), d->newMask);
    formLayout->addRow(trUtf8("New Camera Gateway"), d->newGateway);
    formLayout->addRow(trUtf8("Apply changes"), d->apply);

    setLayout(formLayout);

    connect(d->apply, &QPushButton::clicked, [d]() {
        quint64 mac = static_cast<quint64>(Jgv::Gvcp::DiscoveryAckHelper::deviceMACaddressHigh(d->gvcpUtils->transmitterAck())) << 32;
        mac |= static_cast<quint64>(Jgv::Gvcp::DiscoveryAckHelper::deviceMACaddressLow(d->gvcpUtils->transmitterAck()));
        d->gvcpUtils->forceIP(mac,
                               QHostAddress(d->newIP->text()),
                               QHostAddress(d->newMask->text()),
                               QHostAddress(d->newGateway->text()));
    });
    connect(d->gvcpUtils.data(), &GvcpUtils::forceIpSucces, [d]() {
        d->gvcpUtils->discover(QHostAddress(d->newIP->text()));
    });
    connect(d->gvcpUtils.data(), &GvcpUtils::newDeviceDiscovered, [this, d](const Jgv::Gvcp::DISCOVERY_ACK &ack) {
        d->gvcpUtils->setTransmitterAck(ack);
        this->initializePage();
    });
    connect(d->useForceIP, &QCheckBox::toggled,[d](bool checked) {
        if (checked) {
            d->newIP->setText(QHostAddress(d->gvcpUtils->controller().ip().toIPv4Address() & d->gvcpUtils->controller().netmask().toIPv4Address()).toString());
            d->newMask->setText(d->gvcpUtils->controller().netmask().toString());
            d->newGateway->setText("0.0.0.0");
        } else {
            d->newIP->clear();
            d->newMask->clear();
            d->newGateway->clear();
        }
        d->newIP->setEnabled(checked);
        d->newMask->setEnabled(checked);
        d->newGateway->setEnabled(checked);
        d->apply->setEnabled(checked);
    });
}

void ForceIPPage::initializePage()
{
    Q_D(ForceIPPage);
    d->controllerIp->setText(QString("%0 / %1")
                             .arg(d->gvcpUtils->controller().ip().toString())
                             .arg(d->gvcpUtils->controller().netmask().toString()));
    d->transmeterInfos->setText(ackTohtml(d->gvcpUtils->transmitterAck()));
    d->useForceIP->blockSignals(true);
    d->useForceIP->setChecked(false);
    //forceIPToggled(false);
    d->useForceIP->blockSignals(false);
}

bool ForceIPPage::validatePage()
{
    Q_D(ForceIPPage);
    QHostAddress deviceIP(Jgv::Gvcp::DiscoveryAckHelper::currentIP(d->gvcpUtils->transmitterAck()));
    QHostAddress h1 = d->gvcpUtils->controller().ip();
    int l1 = d->gvcpUtils->controller().prefixLength();
    if (deviceIP.isInSubnet(h1, l1) || d->useForceIP->isChecked())
    {
        d->gvcpUtils->stopBroadcastListener();
        return true;
    }
    return false;
}
