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

#include <client/cache.hpp>
#include <client/preload.hpp>
#include <client/preload_util.hpp>
#include <client/logging.hpp>

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace gkfs::cache {

uint32_t
Cache::gen_dir_id(const std::string& dir_path) {
    return str_hash(dir_path);
}

uint32_t
Cache::get_dir_id(const std::string& dir_path) {
    // check if id already exists in map and return
    if(entry_dir_id_.find(dir_path) != entry_dir_id_.end()) {
        return entry_dir_id_[dir_path];
    }
    // otherwise generate one
    auto dir_id = gen_dir_id(dir_path);
    entry_dir_id_.emplace(dir_path, dir_id);
    return dir_id;
}


void
Cache::insert(const std::string& parent_dir, const std::string name,
              const cache_entry value) {
    std::lock_guard<std::mutex> const lock(mtx_);
    auto dir_id = get_dir_id(parent_dir);
    entries_[dir_id].emplace(name, value);
}

std::optional<cache_entry>
Cache::get(const std::string& parent_dir, const std::string& name) {
    std::lock_guard<std::mutex> const lock(mtx_);
    auto dir_id = get_dir_id(parent_dir);
    if(entries_[dir_id].find(name) != entries_[dir_id].end()) {
        return entries_[dir_id][name];
    } else {
        return {};
    }
}

void
Cache::clear_dir(const std::string& dir_path) {
    std::lock_guard<std::mutex> const lock(mtx_);

    auto id_it = entry_dir_id_.find(dir_path);
    if(id_it == entry_dir_id_.end()) {
        return;
    }
    auto entry_it = entries_.find(id_it->second);
    if(entry_it != entries_.end()) {
        entries_.erase(entry_it);
    }
    entry_dir_id_.erase(id_it);
}

void
Cache::dump_cache_to_log(const std::string& dir_path) {
    std::lock_guard<std::mutex> const lock(mtx_);
    auto id_it = entry_dir_id_.find(dir_path);
    if(id_it == entry_dir_id_.end()) {
        LOG(INFO, "{}(): Cache contents for dir path '{}' NONE", __func__,
            dir_path);
        return;
    }
    auto dir_id = id_it->second;
    for(auto& [name, entry] : entries_[dir_id]) {
        // log entry
        LOG(INFO,
            "{}(): Cache contents for dir path '{}' -> name '{}' is_dir '{}' size '{}' ctime '{}'",
            __func__, dir_path, name,
            entry.file_type == gkfs::filemap::FileType::directory, entry.size,
            entry.ctime);
    }
}

void
Cache::clear() {
    std::lock_guard<std::mutex> const lock(mtx_);
    entries_.clear();
    entry_dir_id_.clear();
}

} // namespace gkfs::cache
