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

#include <daemon/util.hpp>
#include <daemon/daemon.hpp>

#include <common/rpc/rpc_util.hpp>

#include <filesystem> // Added for file existence check
#include <chrono>     // Added for sleep (if needed)
#include <fstream>
#include <iostream>
#include <limits>
#include <thread>

using namespace std;

namespace gkfs::utils {

/**
 * @internal
 * Appends a single line to an existing shared hosts file with the RPC
 * connection information of this daemon. If it doesn't exist, it is created.
 * The line includes the hostname (and rootdir_suffix if applicable) and the RPC
 * server's listening address.
 *
 * NOTE, the shared file system must support strong consistency semantics to
 * ensure each daemon can write its information to the file even if the write
 * access is simultaneous.
 * @endinternal
 */
// void
// populate_hosts_file() {
//     const auto& hosts_file = GKFS_DATA->hosts_file();
//     const auto& daemon_addr = RPC_DATA->self_addr_str();
//     const auto& proxy_addr = RPC_DATA->self_proxy_addr_str();
//     GKFS_DATA->spdlogger()->debug("{}() Populating hosts file: '{}'",
//     __func__,
//                                   hosts_file);
//     ofstream lfstream(hosts_file, ios::out | ios::app);
//     if(!lfstream) {
//         throw runtime_error(fmt::format("Failed to open hosts file '{}': {}",
//                                         hosts_file, strerror(errno)));
//     }
//     // if rootdir_suffix is used, append it to hostname
//     auto hostname =
//             GKFS_DATA->rootdir_suffix().empty()
//                     ? gkfs::rpc::get_my_hostname(true)
//                     : fmt::format("{}#{}", gkfs::rpc::get_my_hostname(true),
//                                   GKFS_DATA->rootdir_suffix());
//     auto line_out = fmt::format("{} {}", hostname, daemon_addr);
//     if(!proxy_addr.empty())
//         line_out = fmt::format("{} {}", line_out, proxy_addr);
//     lfstream << line_out << std::endl;
//
//     if(!lfstream) {
//         throw runtime_error(
//                 fmt::format("Failed to write on hosts file '{}': {}",
//                             hosts_file, strerror(errno)));
//     }
//     lfstream.close();
// }


void
populate_hosts_file() {
    const auto& hosts_file = GKFS_DATA->hosts_file();
    const auto& daemon_addr = RPC_DATA->self_addr_str();
    const auto& proxy_addr = RPC_DATA->self_proxy_addr_str();

    GKFS_DATA->spdlogger()->debug("{}() Populating hosts file: '{}'", __func__,
                                  hosts_file);
    // if rootdir_suffix is used, append it to hostname
    auto hostname =
            GKFS_DATA->rootdir_suffix().empty()
                    ? gkfs::rpc::get_my_hostname(true)
                    : fmt::format("{}#{}", gkfs::rpc::get_my_hostname(true),
                                  GKFS_DATA->rootdir_suffix());
    auto line_out = fmt::format("{} {}", hostname, daemon_addr);
    if(!proxy_addr.empty())
        line_out = fmt::format("{} {}", line_out, proxy_addr);
    // Constants for retry mechanism
    const int MAX_RETRIES = 5; // Maximum number of retry attempts
    const std::chrono::milliseconds RETRY_DELAY(
            3); // Delay between retries (in milliseconds)

    for(int attempt = 1; attempt <= MAX_RETRIES; attempt++) {
        { // New scope to close the file after each write attempt
            std::ofstream lfstream(hosts_file, std::ios::out | std::ios::app);
            if(!lfstream) {
                throw std::runtime_error(
                        fmt::format("Failed to open hosts file '{}': {}",
                                    hosts_file, strerror(errno)));
            }
            lfstream << line_out << std::endl;
            if(!lfstream) {
                throw runtime_error(
                        fmt::format("Failed to write on hosts file '{}': {}",
                                    hosts_file, strerror(errno)));
            }
            lfstream.close();
        } // lfstream closed here

        // Check if the line is in the file
        std::ifstream checkstream(hosts_file);
        std::string line;
        bool lineFound = false;
        while(std::getline(checkstream, line)) {
            if(line == line_out) {
                lineFound = true;
                break;
            }
        }
        checkstream.close();

        if(lineFound) {
            GKFS_DATA->spdlogger()->debug(
                    "{}() Host successfully written and to hosts file",
                    __func__);
            return; // Success, exit the function
        } else {
            GKFS_DATA->spdlogger()->warn(
                    "{}() Host not found after attempt {}, retrying...",
                    __func__, attempt);
            std::this_thread::sleep_for(RETRY_DELAY); // Wait before retrying
        }
    }

    // Failed after all retries
    throw std::runtime_error(fmt::format(
            "Failed to write line to hosts file '{}' after {} retries",
            hosts_file, MAX_RETRIES));
}


/**
 * @internal
 * This function removes the entire hosts file even if just one daemon is
 * shutdown. This makes sense because the data distribution calculation would be
 * misaligned if the entry of the current daemon was only removed.
 * @endinternal
 */
void
destroy_hosts_file() {
    std::remove(GKFS_DATA->hosts_file().c_str());
}

} // namespace gkfs::utils
