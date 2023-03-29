/**  
 *  Copyright 2014 by Benjamin Land (a.k.a. BenLand100)
 *
 *  WbLSdaq is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  WbLSdaq is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WbLSdaq. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ETHERNET_COMMUNICATION_HH__
#define __ETHERNET_COMMUNICATION_HH__

#include <RemoteCommunication.hh>

#include <string>

class EthernetCommunication:public RemoteCommunication {
    public:
        EthernetCommunication(std::string addr, 
                              int port = 1861, 
                              double timeout = 1.0);
        void clear(double timeout = 0.1);
    protected:
        uint8_t seqnum;
        struct timeval timeout; 
};

#endif
