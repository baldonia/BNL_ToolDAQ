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

#include <EthernetCommunication.hh>

#include <cmath>
#include <cstdio>
#include <cstring>

#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <arpa/inet.h>

using namespace std;

EthernetCommunication::EthernetCommunication(string addr, int port, double _timeout) {
    struct hostent *hostent = gethostbyname(addr.c_str());
    if (hostent == NULL) 
        throw runtime_error("Could not find host " + addr);
    
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = hostent->h_addrtype;
    sockaddr.sin_port = htons(port);
    memcpy(&sockaddr.sin_addr, hostent->h_addr, hostent->h_length);
    
    fd = socket(hostent->h_addrtype, SOCK_STREAM, 0);
    if (fd < 0) 
        throw runtime_error("Could not create socket");
    if (connect(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
        throw runtime_error("Could not connect socket " + to_string(errno));
             
    double ipart,fpart;
    fpart = modf(_timeout,&ipart);
    timeout.tv_sec = (int)ipart;
    timeout.tv_usec = round(fpart*1e6);
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) 
        throw runtime_error("Could not set recieve timeout");
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) 
        throw runtime_error("Could not set send timeout");
        
    clear();
    
}

void EthernetCommunication::clear(double _timeout) { 
    double ipart,fpart;
    fpart = modf(_timeout,&ipart);
    struct timeval clrtimeout;
    clrtimeout.tv_sec = (int)ipart;
    clrtimeout.tv_usec = round(fpart*1e6);
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&clrtimeout, sizeof(clrtimeout)) < 0)
        throw runtime_error("Could not set recieve timeout");
    char *buff = new char[4096];
    while (read(fd,buff,4096) > 0) { }
    delete [] buff;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        throw runtime_error("Could not set recieve timeout");
}
