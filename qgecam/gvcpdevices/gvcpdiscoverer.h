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

//#ifndef GVCPDISCOVERER_H
//#define GVCPDISCOVERER_H

#include <memory>

namespace Jgv {

namespace Gvcp {

struct DISCOVERY_ACK;

namespace Discoverer
{
class IListener
{
public:
    virtual ~IListener() = default;
    virtual void deviceDiscovered(const DISCOVERY_ACK &device) = 0;
    virtual void forceIpSucces() = 0;
};
}

class GvcpDiscovererPrivate;
class GvcpDiscoverer
{

public:
    GvcpDiscoverer();
    ~GvcpDiscoverer();

    bool listen(uint32_t srcIP);
    void stop();
    void discover(uint32_t peerIP);
    void forceIP(uint64_t mac, uint32_t newIP, uint32_t newNetmask, uint32_t newGateway);
    void addListener(std::weak_ptr<Discoverer::IListener> listener);

private:
    const std::unique_ptr<GvcpDiscovererPrivate> d_ptr;
    inline GvcpDiscovererPrivate *d_func() { return d_ptr.get(); }
    inline const GvcpDiscovererPrivate *d_func() const { return d_ptr.get(); }
};

} // namespace Gvcp

} // namespace Jgv

//#endif // GVCPDISCOVERER_H
