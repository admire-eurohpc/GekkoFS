/*
  Copyright 2018-2023, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2023, Johannes Gutenberg Universitaet Mainz, Germany

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

#ifndef GKFS_COMMON_MSGPACK_HPP
#define GKFS_COMMON_MSGPACK_HPP

#include <msgpack/msgpack.hpp>
#include <string>
#include <vector>


namespace gkfs::msgpack {

struct client_metrics {
    uint64_t total_bytes_read;
    uint64_t total_bytes_written;


    std::string name;
    int age;
    std::vector<std::string> aliases;

    template<class T>
    void msgpack(T &pack) {
        pack(name, age, aliases);
    }
};

void test_msgpack(){}

}

#endif // GKFS_COMMON_MSGPACK_HPP
