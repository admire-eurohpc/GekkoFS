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

#ifndef GEKKOFS_DAEMON_FORWARD_REDISTRIBUTION_HPP
#define GEKKOFS_DAEMON_FORWARD_REDISTRIBUTION_HPP

#include <daemon/daemon.hpp>
#include <string>

namespace gkfs::malleable::rpc {

int
forward_metadata(std::string& key, std::string& value, unsigned int dest_id);

int
forward_data(const std::string& path, void* buf, const size_t count,
             const uint64_t chnk_id, const uint64_t dest_id);

} // namespace gkfs::malleable::rpc

#endif // GEKKOFS_DAEMON_FORWARD_REDISTRIBUTION_HPP