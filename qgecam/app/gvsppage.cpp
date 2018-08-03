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

#include "gvsppage.h"
#include "gvsppage_p.h"
#include "lineseparator.h"
#include "gvcputils.h"
#include "discoveryhelper.h"

#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QFormLayout>
#include <QNetworkAddressEntry>

using namespace Qroilib;

GvspPage::GvspPage(QSharedPointer<GvcpUtils> discoverer, QWidget *parent)
    : JgvWizardPage(discoverer, *new GvspPagePrivate, parent)
{
    Q_D(GvspPage);
    setTitle(trUtf8("GiGE Vision STREAM"));
    setSubTitle (trUtf8 ("The client of the video stream may be local (on this machine)"
                            "Or remote (another machine)"));
    d->localRadio = new QRadioButton(trUtf8("This machine"));
    d->distRadio = new QRadioButton(trUtf8("Remote machine"));
    d->distRadio->setEnabled(false);
    d->localRadio->setChecked(true);

    d->socketInfo = new QLineEdit;
    d->socketInfo->setReadOnly(true);

    d->discover = new QPushButton(trUtf8("Start"));
    d->discover->setEnabled(false);
    d->receivers = new QComboBox;
    d->receivers->setEnabled(false);
    d->receiverInfos = new QLabel;
    d->receiverInfos->setStyleSheet("background-color: white");
    d->receiverInfos->setEnabled(false);


    QFormLayout *mainLayout = new QFormLayout;
    mainLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    mainLayout->addRow(trUtf8("Receiver Selection"), d->localRadio);
    mainLayout->addWidget(d->distRadio);
    mainLayout->addRow(trUtf8("Information socket GVSP"), d->socketInfo);
    mainLayout->addRow(new LineSeparator);
    mainLayout->addRow(trUtf8("Start a reseach"), d->discover);
    mainLayout->addRow(trUtf8("trays"), d->receivers);
    mainLayout->addRow(trUtf8("Description"), d->receiverInfos);
    setLayout(mainLayout);

    connect(d->discover, &QPushButton::clicked, [d]() {
        d->receivers->setEnabled(false);
        d->receivers->clear();
        d->receiverInfos->clear();
        d->gvcpUtils->discover(d->gvcpUtils->controller().broadcast());
    });

    connect(d->localRadio, &QRadioButton::toggled, [d]() {
        const bool distant = d->distRadio->isChecked();
        d->discover->setEnabled(distant);
        d->receivers->setEnabled(distant);
        d->receiverInfos->setEnabled(distant);
    });
    connect(d->gvcpUtils.data(), &GvcpUtils::newDeviceDiscovered, [d](const Jgv::Gvcp::DISCOVERY_ACK &ack) {
        const quint32 deviceMode = Jgv::Gvcp::DiscoveryAckHelper::deviceMode(ack);
        if (Jgv::Gvcp::DiscoveryAckHelper::deviceClass(deviceMode) == Jgv::Gvcp::enumType(Jgv::Gvcp::DeviceClass::RECEIVER)) {
            QVariant variant;
            variant.setValue(ack);
            d->receivers->addItem(QByteArray(Jgv::Gvcp::DiscoveryAckHelper::modelName(ack), static_cast<uint16_t>(Jgv::Gvcp::BootstrapBlockSize::ModelName)), variant);
            d->receivers->setEnabled(true);
        }
    });

    connect(d->receivers, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [d](int index) {
        if (index >= 0) {
            const Jgv::Gvcp::DISCOVERY_ACK ack = d->receivers->itemData(index).value<Jgv::Gvcp::DISCOVERY_ACK>();
            d->gvcpUtils->setTransmitterAck(ack);
            d->receiverInfos->setText(ackTohtml(ack));
        }
    });

}

GvspPage::~GvspPage()
{}

void GvspPage::initializePage()
{
    Q_D(const GvspPage);

    d->receivers->clear();
    d->receivers->setEnabled(false);

    Jgv::Gvcp::DISCOVERY_ACK transmitter = {};
    //d->gvcpUtils->setTransmitter(transmitter);
    d->receiverInfos->setText(ackTohtml(transmitter));
    d->gvcpUtils->listenForBroadcast();

}

bool GvspPage::validatePage()
{
    Q_D(GvspPage);
//    if (d->withoutDiscovery->isChecked()) {
//        QHostAddress address(d->diffusionIP->text());
//        if (!address.isNull()) {
//            d->gevDescription.gvcp.deviceIP = address.toIPv4Address();
//        } else {
//            d->gevDescription.gvcp.deviceIP = 0;
//        }

//    }
//    return (d->gevDescription.gvcp.deviceIP != 0);
    return true;
}

