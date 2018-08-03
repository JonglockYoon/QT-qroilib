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

#ifndef DISCOVERYPAGE_P_H
#define DISCOVERYPAGE_P_H

#include "jgvwizardpage_p.h"

#include <QHostAddress>

class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;

namespace Qroilib {

class DiscoveryPagePrivate : public JgvWizardPagePrivate
{
public:
    QLabel *controllerIp = nullptr;
    QLabel *transmitterInfos = nullptr;
    QPushButton *discover = nullptr;
    QCheckBox *allNetworks = nullptr;
    QComboBox *devicesList = nullptr;
    QHostAddress diffusion;


};

} // namespace Qroilib;

Q_DECLARE_METATYPE(Jgv::Gvcp::DISCOVERY_ACK)

#endif // DISCOVERYPAGE_P_H
