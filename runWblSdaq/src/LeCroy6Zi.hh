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

#include <cstdio>
#include <cstring>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <stdexcept>

#include <RemoteCommunication.hh>

class LeCroy6Zi {
    public:
        LeCroy6Zi(RemoteCommunication* remote);
        virtual ~LeCroy6Zi();
        
        RemoteCommunication* remote;
        
        void checklast();
        
        inline void stop() { remote->send("TRMD STOP"); }
        inline void normal() { remote->send("TRMD NORM"); }
        inline void single() { remote->send("TRMD SINGLE"); }
        inline void save(int i) { remote->send("*SAV " + std::to_string(i)); }
        inline void recall(int i) { remote->send("*RCL " + std::to_string(i)); }
        inline void reset() { remote->send("*RST"); }
};
