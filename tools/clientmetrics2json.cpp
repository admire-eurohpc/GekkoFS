/*
  Copyright 2018-2023, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2023, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS' POSIX interface.

  GekkoFS' POSIX interface is free software: you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  GekkoFS' POSIX interface is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with GekkoFS' POSIX interface.  If not, see
  <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: LGPL-3.0-or-later
*/

#include <common/msgpack_util.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
}

using namespace std;
using json = nlohmann::json;
int
main() {

    std::ifstream file(
            "/tmp/gkfs_client_metrics/_2023-12-06_17:43:11_evie_365368.msgpack",
            std::ios::binary);
    if(!file.is_open()) {
        std::cout << "failed to open " << '\n';
        return -1;
    }
    std::vector<unsigned char> const buffer(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

    file.close();
    std::error_code ec{};
    auto undata = msgpack::unpack<gkfs::messagepack::ClientMetrics>(buffer, ec);

    json json_obj;
    json_obj["hostname"] = undata.hostname_;
    json_obj["pid"] = undata.pid_;
    json_obj["total_bytes"] = undata.total_bytes_;
    json_obj["total_iops"] = undata.total_iops_;
    json_obj["start_t"] = undata.start_t_;
    json_obj["end_t"] = undata.end_t_;
    json_obj["avg"] = undata.avg_;

    std::cout << "Generated JSON:" << std::endl;
    std::cout << json_obj.dump(4) << std::endl; // Use dump for pretty printing

    return 0;
}