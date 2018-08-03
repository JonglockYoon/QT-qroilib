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

#ifndef JGVWIZARDPAGE_H
#define JGVWIZARDPAGE_H

#include <QWizardPage>

namespace Jgv {
namespace Gvcp {
struct DISCOVERY_ACK;
}
}

namespace Qroilib {



class GvcpUtils;

class JgvWizardPagePrivate;
class JgvWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    virtual ~JgvWizardPage();
    static QString ackTohtml(const Jgv::Gvcp::DISCOVERY_ACK &ack);

protected:
    explicit JgvWizardPage(QSharedPointer<GvcpUtils> discoverer, JgvWizardPagePrivate &dd, QWidget *parent);
    const QScopedPointer<JgvWizardPagePrivate> d_ptr;

private:
    Q_DECLARE_PRIVATE(JgvWizardPage)
    Q_DISABLE_COPY(JgvWizardPage)

};

} // namespace Qroilib

#endif // JGVWIZARDPAGE_H
