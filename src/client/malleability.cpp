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

#include <client/user_functions.hpp>
#include <client/rpc/forward_malleability.hpp>
#include <client/logging.hpp>

namespace gkfs::malleable {

int
expand_start(int old_server_conf, int new_server_conf) {
    LOG(INFO, "{}() Expand operation enter", __func__);
    gkfs::malleable::rpc::forward_expand_start(old_server_conf,
                                               new_server_conf);
    return 0;
}

int
expand_status() {
    LOG(INFO, "{}() Expand operation status", __func__);
    return gkfs::malleable::rpc::forward_expand_status();
}

int
expand_finalize() {
    LOG(INFO, "{}() Expand operation finalize", __func__);
    return gkfs::malleable::rpc::forward_expand_finalize();
}

} // namespace gkfs::malleable