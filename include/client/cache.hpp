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

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>

namespace gkfs::cache {

class Cache {
private:
    std::unordered_map<std::string, std::string> entries_;
    std::mutex mtx_;

public:
    Cache() = default;

    virtual ~Cache() = default;

    void
    insert(const std::string& key, const std::string& value);

    std::optional<std::string>
    get(const std::string& key);

    void
    remove(const std::string& key);

    void
    clear();
};

} // namespace gkfs::cache

#endif // GKFS_CLIENT_CACHE
