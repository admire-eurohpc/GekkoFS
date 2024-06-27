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
#include <mutex>
#include <optional>
#include <string>

namespace gkfs::cache {

void
Cache::insert(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> const lock(mtx_);
    entries_[key] = value;
}

std::optional<std::string>
Cache::get(const std::string& key) {
    std::lock_guard<std::mutex> const lock(mtx_);
    // return key if found
    if(entries_.find(key) != entries_.end()) {
        return entries_[key];
    }
    return {};
}

void
Cache::remove(const std::string& key) {
    std::lock_guard<std::mutex> const lock(mtx_);
    entries_.erase(key);
}

void
Cache::clear() {
    std::lock_guard<std::mutex> const lock(mtx_);
    entries_.clear();
}

} // namespace gkfs::cache
