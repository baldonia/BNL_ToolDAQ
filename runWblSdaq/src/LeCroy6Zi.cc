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

#include "LeCroy6Zi.hh"

#include <cmath>
#include <iostream>
#include <arpa/inet.h>

using namespace std;

LeCroy6Zi::LeCroy6Zi(RemoteCommunication* _remote){
    this->remote = _remote;
}

LeCroy6Zi::~LeCroy6Zi() {
    delete remote;
}

void LeCroy6Zi::checklast() {
    remote->send("cmr?");
    string response = remote->recv();
    if (response.size() > 4) {
        unsigned long code = strtoul(&response[5],NULL,10);
        switch (code) {
            case 0:  return;
            case 1:  throw runtime_error("unrecognized command/query header");
            case 2:  throw runtime_error("illegal header path");
            case 3:  throw runtime_error("illegal number");
            case 4:  throw runtime_error("illegal number suffix");
            case 5:  throw runtime_error("unrecognized keyword");
            case 6:  throw runtime_error("string error");
            case 7:  throw runtime_error("GET embedded in another message");
            case 10: throw runtime_error("arbitrary data block expected");
            case 11: throw runtime_error("non-digit character in byte count field of arbitrary data block");
            case 12: throw runtime_error("EOI detected during definite length data block transfer");
            case 13: throw runtime_error("extra bytes detected during definite length data block transfer");
            default: throw runtime_error("Unknown response from CMR?");
        }
    } else {
        throw runtime_error("Invalid response to CMR?");
    }
}
