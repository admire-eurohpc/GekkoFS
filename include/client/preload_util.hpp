/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

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

#ifndef GEKKOFS_PRELOAD_UTIL_HPP
#define GEKKOFS_PRELOAD_UTIL_HPP

#include <client/preload.hpp>
#include <common/metadata.hpp>
#include <string>
#include <iostream>
#include <map>
#include <type_traits>
#include <optional>

namespace gkfs::metadata {

struct MetadentryUpdateFlags {
    bool atime = false;
    bool mtime = false;
    bool ctime = false;
    bool uid = false;
    bool gid = false;
    bool mode = false;
    bool link_count = false;
    bool size = false;
    bool blocks = false;
    bool path = false;
};

} // namespace gkfs::metadata

// Hermes instance
namespace hermes {
class async_engine;
}

extern std::unique_ptr<hermes::async_engine> ld_network_service;
extern std::unique_ptr<hermes::async_engine> ld_proxy_service;

// function definitions
namespace gkfs::utils {
template <typename E>
constexpr typename std::underlying_type<E>::type
to_underlying(E e) {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

std::optional<gkfs::metadata::Metadata>
get_metadata(const std::string& path, bool follow_links = false);

int
metadata_to_stat(const std::string& path, const gkfs::metadata::Metadata& md,
                 struct stat& attr);

/**
 * @brief Updates write size on the metadata daemon
 * @param path
 * @param count
 * @param offset
 * @param is_append
 * @return <err, return_offset>
 */
std::pair<int, off64_t>
update_file_size(const std::string& path, size_t count, off64_t offset,
                 bool is_append);

void
load_hosts();

void
load_forwarding_map();

std::vector<std::pair<std::string, std::string>>
read_hosts_file();

void
connect_to_hosts(const std::vector<std::pair<std::string, std::string>>& hosts);

void
check_for_proxy();

void
lookup_proxy_addr();

} // namespace gkfs::utils


#endif // GEKKOFS_PRELOAD_UTIL_HPP
