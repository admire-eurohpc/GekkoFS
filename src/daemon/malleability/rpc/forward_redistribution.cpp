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
#include "common/rpc/rpc_util.hpp"


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

int
forward_data(const std::string& path, void* buf, const size_t count,
             const uint64_t chnk_id, const uint64_t dest_id) {
    hg_handle_t rpc_handle = nullptr;
    rpc_write_data_in_t in{};
    rpc_data_out_t out{};
    int err = 0;
    in.path = path.c_str();
    in.offset = 0; // relative to chunkfile not gkfs file
    in.host_id = dest_id;
    in.host_size = RPC_DATA->distributor()->hosts_size();
    in.chunk_n = 1;
    in.chunk_start = chnk_id;
    in.chunk_end = chnk_id;
    in.total_chunk_size = count;
    std::vector<uint8_t> write_ops_vect = {1};
    in.wbitset = gkfs::rpc::compress_bitset(write_ops_vect).c_str();

    hg_bulk_t bulk_handle = nullptr;
    // register local target buffer for bulk access
    auto bulk_buf = buf;
    auto size = std::make_shared<size_t>(count); // XXX Why shared ptr?
    auto ret = margo_bulk_create(RPC_DATA->client_rpc_mid(), 1, &bulk_buf,
                                 size.get(), HG_BULK_READ_ONLY, &bulk_handle);
    if(ret != HG_SUCCESS) {
        GKFS_DATA->spdlogger()->error("{}() Failed to create rpc bulk handle",
                                      __func__);
        return EBUSY;
    }
    in.bulk_handle = bulk_handle;
    GKFS_DATA->spdlogger()->trace(
            "{}() Sending non-blocking RPC to '{}': path '{}' offset '{}' chunk_n '{}' chunk_start '{}' chunk_end '{}' total_chunk_size '{}'",
            __func__, dest_id, in.path, in.offset, in.chunk_n, in.chunk_start,
            in.chunk_end, in.total_chunk_size);
    ret = margo_create(RPC_DATA->client_rpc_mid(),
                       RPC_DATA->rpc_endpoints().at(dest_id),
                       RPC_DATA->rpc_client_ids().migrate_data_id, &rpc_handle);
    if(ret != HG_SUCCESS) {
        margo_destroy(rpc_handle);
        margo_bulk_free(bulk_handle);
        return EBUSY;
    }
    // Send RPC
    ret = margo_forward(rpc_handle, &in);
    if(ret != HG_SUCCESS) {
        GKFS_DATA->spdlogger()->error(
                "{}() Unable to send blocking rpc for path {} and recipient {}",
                __func__, path, dest_id);
        margo_destroy(rpc_handle);
        margo_bulk_free(bulk_handle);
        return EBUSY;
    }
    GKFS_DATA->spdlogger()->debug("{}() '1' RPCs sent, waiting for reply ...",
                                  __func__);
    ssize_t out_size = 0;
    ret = margo_get_output(rpc_handle, &out);
    if(ret != HG_SUCCESS) {
        GKFS_DATA->spdlogger()->error(
                "{}() Failed to get rpc output for path {} recipient {}",
                __func__, path, dest_id);
        err = EBUSY;
    }
    GKFS_DATA->spdlogger()->debug(
            "{}() Got response from target '{}': err '{}' with io_size '{}'",
            __func__, dest_id, out.err, out.io_size);
    if(out.err != 0)
        err = out.err;
    else
        out_size += static_cast<size_t>(out.io_size);
    margo_free_output(rpc_handle, &out);
    margo_destroy(rpc_handle);
    margo_bulk_free(bulk_handle);
    return err;
}

} // namespace gkfs::malleable::rpc