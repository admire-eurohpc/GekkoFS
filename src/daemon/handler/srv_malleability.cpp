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
#include <daemon/daemon.hpp>
#include <daemon/handler/rpc_defs.hpp>
#include <daemon/malleability/malleable_manager.hpp>

#include <common/rpc/rpc_types.hpp>

extern "C" {
#include <unistd.h>
}

using namespace std;

namespace {

hg_return_t
rpc_srv_expand_start(hg_handle_t handle) {
    rpc_expand_start_in_t in;
    rpc_err_out_t out;

    auto ret = margo_get_input(handle, &in);
    if(ret != HG_SUCCESS)
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to retrieve input from handle", __func__);
    assert(ret == HG_SUCCESS);
    GKFS_DATA->spdlogger()->debug(
            "{}() Got RPC with old conf '{}' new conf '{}'", __func__,
            in.old_server_conf, in.new_server_conf);
    try {
        // if maintenance mode is already set, error is thrown -- not allowed
        GKFS_DATA->maintenance_mode(true);
        GKFS_DATA->malleable_manager()->expand_start(in.old_server_conf,
                                                     in.new_server_conf);
        out.err = 0;
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error("{}() Failed to start expansion: '{}' ",
                                      __func__, e.what());
        out.err = -1;
    }

    GKFS_DATA->spdlogger()->debug("{}() Sending output err '{}'", __func__,
                                  out.err);
    auto hret = margo_respond(handle, &out);
    if(hret != HG_SUCCESS) {
        GKFS_DATA->spdlogger()->error("{}() Failed to respond", __func__);
    }
    // Destroy handle when finished
    margo_free_input(handle, &in);
    margo_destroy(handle);
    return HG_SUCCESS;
}

hg_return_t
rpc_srv_expand_status(hg_handle_t handle) {
    rpc_err_out_t out;
    GKFS_DATA->spdlogger()->debug("{}() Got RPC ", __func__);
    try {
        // return 1 if redistribution is running, 0 otherwise.
        out.err = GKFS_DATA->redist_running() ? 1 : 0;
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to check status for expansion: '{}'", __func__,
                e.what());
        out.err = -1;
    }
    GKFS_DATA->spdlogger()->debug("{}() Sending output err '{}'", __func__,
                                  out.err);
    auto hret = margo_respond(handle, &out);
    if(hret != HG_SUCCESS) {
        GKFS_DATA->spdlogger()->error("{}() Failed to respond", __func__);
    }
    // Destroy handle when finished
    margo_destroy(handle);
    return HG_SUCCESS;
}

hg_return_t
rpc_srv_expand_finalize(hg_handle_t handle) {
    rpc_err_out_t out;
    GKFS_DATA->spdlogger()->debug("{}() Got RPC ", __func__);
    try {
        GKFS_DATA->maintenance_mode(false);
        out.err = 0;
    } catch(const std::exception& e) {
        GKFS_DATA->spdlogger()->error("{}() Failed to finalize expansion: '{}'",
                                      __func__, e.what());
        out.err = -1;
    }

    GKFS_DATA->spdlogger()->debug("{}() Sending output err '{}'", __func__,
                                  out.err);
    auto hret = margo_respond(handle, &out);
    if(hret != HG_SUCCESS) {
        GKFS_DATA->spdlogger()->error("{}() Failed to respond", __func__);
    }

    // Destroy handle when finished
    margo_destroy(handle);
    return HG_SUCCESS;
}

} // namespace

DEFINE_MARGO_RPC_HANDLER(rpc_srv_expand_start)

DEFINE_MARGO_RPC_HANDLER(rpc_srv_expand_status)

DEFINE_MARGO_RPC_HANDLER(rpc_srv_expand_finalize)
