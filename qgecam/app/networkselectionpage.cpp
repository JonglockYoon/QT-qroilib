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

#include "networkselectionpage.h"
#include "networkselectionpage_p.h"
#include "lineseparator.h"

#include "gvcputils.h"

#include <QFormLayout>
#include <QPushButton>
#include <cstring>

using namespace Qroilib;

NetworkSelectionPage::NetworkSelectionPage(QSharedPointer<GvcpUtils> discoverer, QWidget *parent)
    : JgvWizardPage(discoverer, *new NetworkSelectionPagePrivate, parent)
{
    Q_D(NetworkSelectionPage);
    setTitle(trUtf8("NETWORK"));
    setSubTitle (trUtf8 ("Choose here on which network interface jiguiviou will communicate"
                            "With the Giage Vision camera"));

    QPushButton *get = new QPushButton(trUtf8("Refresh"));

    d->mainLayout = new QFormLayout;
    d->mainLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    d->mainLayout->addRow(trUtf8("Update interfaces"), get);
    d->mainLayout->addRow(new LineSeparator);
    setLayout(d->mainLayout);

    connect(get, &QPushButton::clicked, [this]() {
        this->clearInterfaces();
        this->populateInterfaces();
    });
}

NetworkSelectionPage::~NetworkSelectionPage()
{}

void NetworkSelectionPage::populateInterfaces()
{
    Q_D(NetworkSelectionPage);
    foreach (QNetworkInterface ni, QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry ae, ni.addressEntries()) {
            if ((ae.ip().protocol()==QAbstractSocket::IPv4Protocol)
                    && (ae.ip() != QHostAddress::LocalHost)) {
                NetworkInterfaceRadioButton *interface = new NetworkInterfaceRadioButton(ae, ni);
                d->interfaces.append(interface);
                if (d->interfaces.count() == 1) {
                    interface->setChecked(true);
                    d->mainLayout->addRow("Available Interfaces", interface);
                }
                else {
                    d->mainLayout->addWidget(interface);
                }
            }
        }
    }
}

void NetworkSelectionPage::clearInterfaces()
{
    Q_D(NetworkSelectionPage);
    foreach (QWidget *w, d->interfaces) {
        int row;
        QFormLayout::ItemRole role;
        d->mainLayout->getWidgetPosition(w, &row, &role);
        QLayoutItem *item = d->mainLayout->itemAt(row, role);
        d->mainLayout->removeItem(item);
        QLayoutItem *label = d->mainLayout->itemAt(row, QFormLayout::LabelRole);
        if (label != NULL) {
            d->mainLayout->removeItem(label);
            delete label->widget();
            delete label;
        }
    }
    qDeleteAll(d->interfaces);
    d->interfaces.clear();
    d->gvcpUtils->setController(QNetworkAddressEntry());
}

void NetworkSelectionPage::initializePage()
{
    clearInterfaces();
    populateInterfaces();
}

bool NetworkSelectionPage::validatePage()
{
    Q_D(NetworkSelectionPage);

    auto it = d->interfaces.constBegin();
    for (; it != d->interfaces.constEnd(); ++it) {
        if ((*it)->isChecked()) {
            d->gvcpUtils->setController((*it)->address());
            return true;
        }
    }

    return false;
}
