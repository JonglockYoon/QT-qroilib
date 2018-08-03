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

#ifndef DISCOVERYPAGE_H
#define DISCOVERYPAGE_H

#include "jgvwizardpage.h"

namespace Qroilib {

class DiscoveryPagePrivate;
class DiscoveryPage : public JgvWizardPage
{

public:
    explicit DiscoveryPage(QSharedPointer<GvcpUtils> discoverer, QWidget *parent = nullptr);
    virtual ~DiscoveryPage();

private:
    void initializePage() override;
    bool validatePage() override;
    void cleanupPage() override;

private:
    Q_DECLARE_PRIVATE(DiscoveryPage)

};

} // namespace Qroilib

#endif // DISCOVERYPAGE_H
