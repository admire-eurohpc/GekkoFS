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
#ifndef GEKKOFS_DAEMON_MALLEABLE_MANAGER_HPP
#define GEKKOFS_DAEMON_MALLEABLE_MANAGER_HPP

#include <daemon/daemon.hpp>

namespace gkfs::malleable {

class MalleableManager {
private:
    ABT_thread redist_thread_;

    // TODO next 3 functions are mostly copy paste from preload_util. FIX

    std::vector<std::pair<std::string, std::string>>
    load_hostfile(const std::string& path);

    std::vector<std::pair<std::string, std::string>>
    read_hosts_file();

    void
    connect_to_hosts(
            const std::vector<std::pair<std::string, std::string>>& hosts);

    int
    redistribute_metadata();

    void
    redistribute_data();

    static void
    expand_abt(void* _arg);

public:
    void
    expand_start(int old_server_conf, int new_server_conf);
};
} // namespace gkfs::malleable


#endif // GEKKOFS_MALLEABLE_MANAGER_HPP
