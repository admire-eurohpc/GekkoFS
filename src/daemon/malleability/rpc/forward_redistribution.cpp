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

#include <daemon/malleability/rpc/forward_redistribution.hpp>
#include <common/rpc/rpc_types.hpp>


namespace gkfs::malleable::rpc {

int
forward_metadata(std::string& key, std::string& value, unsigned int dest_id) {
    hg_handle_t rpc_handle = nullptr;
    rpc_migrate_metadata_in_t in{};
    rpc_err_out_t out{};
    int err;
    // set input
    in.key = key.c_str();
    in.value = value.c_str();
    // Create handle
    GKFS_DATA->spdlogger()->debug("{}() Creating Margo handle ...", __func__);
    auto endp = RPC_DATA->rpc_endpoints().at(dest_id);
    auto ret = margo_create(RPC_DATA->client_rpc_mid(), endp,
                            RPC_DATA->rpc_client_ids().migrate_metadata_id,
                            &rpc_handle);
    if(ret != HG_SUCCESS) {
        GKFS_DATA->spdlogger()->error(
                "{}() Critical error. Cannot create margo handle", __func__);
        return EBUSY;
    }
    ret = margo_forward(rpc_handle, &in);
    if(ret == HG_SUCCESS) {
        // Get response
        GKFS_DATA->spdlogger()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(rpc_handle, &out);
        if(ret == HG_SUCCESS) {
            GKFS_DATA->spdlogger()->debug("{}() Got response success: {}",
                                          __func__, out.err);
            err = out.err;
            margo_free_output(rpc_handle, &out);
        } else {
            // something is wrong
            err = EBUSY;
            GKFS_DATA->spdlogger()->error("{}() while getting rpc output",
                                          __func__);
        }
    } else {
        // something is wrong
        err = EBUSY;
        GKFS_DATA->spdlogger()->error("{}() sending rpc failed", __func__);
    }

    /* clean up resources consumed by this rpc */
    margo_destroy(rpc_handle);
    return err;
}

void
forward_data() {}

} // namespace gkfs::malleable::rpc