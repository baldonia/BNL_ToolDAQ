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

#include <IOUtils.hh>
#include <RemoteCommunication.hh>

#include <cerrno>
#include <cstdio>
#include <cstring>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <stdexcept>

RemoteCommunication::~RemoteCommunication() {
    close(fd);
}

void RemoteCommunication::send(std::string msg, uint8_t flags) {
//  header h;
//  h.operation = flags;
//  h.version = 1;
//  h.seqnum = ((seqnum++) % 254) + 1;
//  h.spare = 0;
//  h.length = htonl(msg.size()+1);
    
    //cout << "<<< " << msg << endl;
    
//  fdwrite(fd,&h,sizeof(h));
    fdwrite(fd,msg.c_str(),msg.size());
    fdwrite(fd,"\n",1);
}

std::string RemoteCommunication::recv() {
    header h;
    std::vector<uint8_t> buf;
    do {
        fdread(fd,&h,sizeof(h));
        h.length = ntohl(h.length);
        size_t off = buf.size();
        buf.resize(off+h.length);
        fdread(fd,&buf[off],h.length);
    } while (!(h.operation & OP_EOI));
    buf[h.length-1] = '\0';
    
    //cout << ">>> " << (const char*)&buf[0] << endl;
   
    return std::string((const char*)&buf[0]);
}
