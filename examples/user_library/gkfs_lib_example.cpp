/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

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
#include <client/user_functions.hpp>

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

extern "C" {
#include <fcntl.h>
#include <stdlib.h>
}

using namespace std;

void
write_file(std::string filename) {
    // Open File
    int fd = gkfs::syscall::gkfs_open(filename, S_IRWXU, O_RDWR | O_CREAT);

    cout << "FD open  " << fd << endl;

    char* bufwrite = (char*) malloc(10);
    strncpy(bufwrite, "testing", 8);

    int size = gkfs::syscall::gkfs_write(fd, bufwrite, 7);

    cout << "Written size " << size << endl;

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


    write_file("/secondfile.tmp");

    auto f_list = gkfs::syscall::gkfs_get_file_list("/");

    for(auto f : f_list) {
        cout << "File: " << f << endl;
        struct stat buf;
        memset(&buf, 0, sizeof(struct stat));

        gkfs::syscall::gkfs_stat("/" + f, &buf, true);

        cout << "Size: " << buf.st_size << " Mode: " << buf.st_mode << endl;
        cout << "Atime: " << buf.st_atime << " Mtime: " << buf.st_mtime
             << " Ctime: " << buf.st_ctime << endl
             << " ****** " << endl;
    }

    res = gkfs_end();

    cout << "End result " << res << endl;
}
