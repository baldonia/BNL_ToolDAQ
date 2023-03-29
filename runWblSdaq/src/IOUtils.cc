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

void fdwrite(int fd, const void *buf, size_t count) {
    size_t num = 0;
    while (num != count) {
        int res = write(fd,((char*)buf)+num,count-num);
        if (res == -1){
            perror("error is: ");
            throw std::runtime_error("Write failed " + std::to_string(errno));
        }
        num += res;
    }
}

void fdread(int fd, void *buf, size_t count) {
    size_t num = 0;
    while (num != count) {
        int res = read(fd,((char*)buf)+num,count-num);
        if (res == -1){
            perror("error is: ");
            throw std::runtime_error("Read failed " + std::to_string(errno));
        }
        if (res == 0){
            perror("error is: ");
            throw std::runtime_error("EOF Reached");
        }
        num += res;
    }
}
