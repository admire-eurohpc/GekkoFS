/*
  Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS.

  GekkoFS is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  GekkoFS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: GPL-3.0-or-later
*/


#include <cmath>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <queue>
#include <regex.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits>
#include <cstdint>
#include <client/user_functions.hpp>

using namespace std;

void
write_file(std::string filename) {
    // Open File
    int fd = gkfs::syscall::gkfs_open(filename, S_IRWXU, O_RDWR | O_CREAT);

    cout << "FD open  " << fd << endl;

    char* bufwrite = (char*) malloc(10);
    strncpy(bufwrite, "testing", 7);


    int size = gkfs::syscall::gkfs_write(fd, bufwrite, 7);

    cout << "Writting size " << size << endl;

    free(bufwrite);
    gkfs::syscall::gkfs_close(fd);
}


void
read_file(std::string filename) {
    int fdread = gkfs::syscall::gkfs_open(filename, S_IRWXU, O_RDONLY);
    if(fdread == -1)
        return;
    char* bufread = (char*) malloc(10);
    int sizeread = gkfs::syscall::gkfs_read(fdread, bufread, 7);

    cout << "Reading Size: " << sizeread << " Content: " << bufread << endl;

    free(bufread);
    gkfs::syscall::gkfs_close(fdread);
}

int
main(int argc, char** argv) {
    cout << "GekkoFS Client library test" << endl;

    auto res = gkfs_init();

    cout << "Init result " << res << endl;

    write_file("/test.tmp");

    read_file("/test.tmp");

    res = gkfs_end();

    cout << "End result " << res << endl;
}
