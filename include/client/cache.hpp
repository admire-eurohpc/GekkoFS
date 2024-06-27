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

#ifndef GKFS_CLIENT_CACHE
#define GKFS_CLIENT_CACHE

#include <client/open_file_map.hpp>

#include <ctime>
#include <functional>
#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <cstdint>

namespace gkfs::cache {

namespace dir {

struct cache_entry {
    gkfs::filemap::FileType file_type;
    uint64_t size;
    time_t ctime;
};

class DentryCache {
private:
    // <dir_id, <name, cache_entry>>
    std::unordered_map<uint32_t, std::unordered_map<std::string, cache_entry>>
            entries_;
    // <dir_path, dir_id>
    std::unordered_map<std::string, uint32_t> entry_dir_id_;
    std::mutex mtx_;
    std::hash<std::string> str_hash;

    uint32_t
    gen_dir_id(const std::string& dir_path);

    uint32_t
    get_dir_id(const std::string& dir_path);

public:
    DentryCache() = default;

    virtual ~DentryCache() = default;

    void
    insert(const std::string& parent_dir, const std::string name,
           const cache_entry value);

    std::optional<cache_entry>
    get(const std::string& parent_dir, const std::string& name);

    void
    clear_dir(const std::string& dir_path);

    void
    dump_cache_to_log(const std::string& dir_path);

    void
    clear();
};
} // namespace dir

//// <path<cnt, size>>
// std::unordered_map<std::string, std::pair<size_t, size_t>> size_cache;


} // namespace gkfs::cache

#endif // GKFS_CLIENT_CACHE
