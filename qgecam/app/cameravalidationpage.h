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

#ifndef CAMERAVALIDATIONPAGE_H
#define CAMERAVALIDATIONPAGE_H

#include "jgvwizardpage.h"

namespace Qroilib {

class CameraValidationPagePrivate;
class CameraValidationPage : public JgvWizardPage
{

public:
    explicit CameraValidationPage(QSharedPointer<GvcpUtils> discoverer, QWidget *parent = nullptr);
    virtual ~CameraValidationPage();

private:
    void initializePage() override;
    bool validatePage() override;
    void cleanupPage() override;
    int nextId() const override;

    Q_DECLARE_PRIVATE(CameraValidationPage)

};

} // namespace Qroilib

#endif // CAMERAVALIDATIONPAGE_H
