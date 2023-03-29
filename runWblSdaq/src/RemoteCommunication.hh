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

#ifndef __REMOTE_COMMUNICATION_HH__
#define __REMOTE_COMMUNICATION_HH__

#include <string>

// abstract base class
class RemoteCommunication {
    public:
        RemoteCommunication() : seqnum(0) { /* */ };
       ~RemoteCommunication();
        void send(std::string msg, uint8_t flags = OP_DATA|OP_EOI|OP_LOCKOUT);
        std::string recv();

    protected:
        static constexpr uint8_t OP_DATA    = 1<<7;
        static constexpr uint8_t OP_REMOTE  = 1<<6;
        static constexpr uint8_t OP_LOCKOUT = 1<<5;
        static constexpr uint8_t OP_CLEAR   = 1<<4;
        static constexpr uint8_t OP_SRQ     = 1<<3;
        static constexpr uint8_t OP_SPOLL   = 1<<2;
        static constexpr uint8_t OP_RES     = 1<<1;
        static constexpr uint8_t OP_EOI     = 1<<0;
        
        typedef struct {
            uint8_t operation;
            uint8_t version;
            uint8_t seqnum;
            uint8_t spare;
            uint32_t length;
        } header;

        int fd;
        int seqnum;
};

#endif
