/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <proxy/proxy.hpp>
#include <proxy/rpc/rpc_defs.hpp>
#include <proxy/rpc/forward_data.hpp>
#include <proxy/rpc/rpc_util.hpp>

#include <common/rpc/rpc_types.hpp>

using namespace std;

/**
 * RPC handler for an incoming write RPC
 * @param handle
 * @return
 */
static hg_return_t
proxy_rpc_srv_write(hg_handle_t handle) {

    rpc_client_proxy_write_in_t client_in{};
    rpc_data_out_t client_out{};
    client_out.err = EIO;
    client_out.io_size = 0;
    hg_bulk_t bulk_handle = nullptr;
    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        client_out.err = EBUSY;
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_handle_get_instance(handle);
    auto bulk_size = margo_bulk_get_size(client_in.bulk_handle);
    assert(bulk_size == client_in.write_size);
    PROXY_DATA->log()->debug(
            "{}() Got RPC with path '{}' bulk_size '{}' == write_size '{}'",
            __func__, client_in.path, bulk_size, client_in.write_size);
    /*
     * Set up buffer and pull from client
     */
    void* bulk_buf; // buffer for bulk transfer
    // create bulk handle and allocated memory for buffer with buf_sizes
    // information
    ret = margo_bulk_create(mid, 1, nullptr, &bulk_size, HG_BULK_READWRITE,
                            &bulk_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to create bulk handle", __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    // access the internally allocated memory buffer and put it into buf_ptrs
    uint32_t actual_count; // number of segments. we use one here because we
                           // pull the whole buffer at once
    ret = margo_bulk_access(bulk_handle, 0, bulk_size, HG_BULK_READWRITE, 1,
                            &bulk_buf, &bulk_size, &actual_count);
    if(ret != HG_SUCCESS || actual_count != 1) {
        PROXY_DATA->log()->error(
                "{}() Failed to access allocated buffer from bulk handle",
                __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                          &bulk_handle);
    }
    // pull data from client here
    ret = margo_bulk_transfer(mid, HG_BULK_PULL, hgi->addr,
                              client_in.bulk_handle, 0, bulk_handle, 0,
                              bulk_size);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error(
                "{}() Failed to pull data from client for path '{}' with size '{}'",
                __func__, client_in.path, bulk_size);
        client_out.err = EBUSY;
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                          &bulk_handle);
    }

    // Forward request to daemon, using bulk_buf, containing the pulled data
    // (which is pulled again by the daemon)
    auto daemon_out = gkfs::rpc::forward_write(client_in.path, bulk_buf,
                                               client_in.offset, bulk_size);
    client_out.err = daemon_out.first;
    client_out.io_size = daemon_out.second;
    PROXY_DATA->log()->debug("{}() Sending output err '{}' io_size '{}'",
                             __func__, client_out.err, client_out.io_size);

    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                      &bulk_handle);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_write)

static hg_return_t
proxy_rpc_srv_read(hg_handle_t handle) {
    rpc_client_proxy_read_in_t client_in{};
    rpc_data_out_t client_out{};
    client_out.err = EIO;
    client_out.io_size = 0;
    hg_bulk_t bulk_handle = nullptr;
    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        client_out.err = EBUSY;
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_handle_get_instance(handle);
    auto bulk_size = margo_bulk_get_size(client_in.bulk_handle);
    assert(bulk_size == client_in.read_size);
    PROXY_DATA->log()->debug(
            "{}() Got RPC with path '{}' bulk_size '{}' == read_size '{}'",
            __func__, client_in.path, bulk_size, client_in.read_size);
    /*
     * Set up buffer for push from daemon
     */
    void* bulk_buf; // buffer for bulk transfer
    // create bulk handle and allocated memory for buffer with buf_sizes
    // information
    ret = margo_bulk_create(mid, 1, nullptr, &bulk_size, HG_BULK_READWRITE,
                            &bulk_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to create bulk handle", __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    // access the internally allocated memory buffer
    uint32_t actual_count; // number of segments. we use one here because we
                           // pull the whole buffer at once
    ret = margo_bulk_access(bulk_handle, 0, bulk_size, HG_BULK_READWRITE, 1,
                            &bulk_buf, &bulk_size, &actual_count);
    if(ret != HG_SUCCESS || actual_count != 1) {
        PROXY_DATA->log()->error(
                "{}() Failed to access allocated buffer from bulk handle",
                __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                          &bulk_handle);
    }
    // Forward request to daemon, using bulk_buf, containing the allocated
    // buffer (which is pushed the data by the daemon)
    auto daemon_out = gkfs::rpc::forward_read(client_in.path, bulk_buf,
                                              client_in.offset, bulk_size);
    if(daemon_out.first != 0) {
        PROXY_DATA->log()->error(
                "{}() Failure when forwarding to daemon with err '{}' and iosize '{}'",
                __func__, daemon_out.first, daemon_out.second);
        client_out.err = daemon_out.first;
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                          &bulk_handle);
    }
    // Push data to client here if no error was reported by the daemon
    ret = margo_bulk_transfer(mid, HG_BULK_PUSH, hgi->addr,
                              client_in.bulk_handle, 0, bulk_handle, 0,
                              bulk_size);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error(
                "{}() Failed to push data from client for path '{}' with size '{}'",
                __func__, client_in.path, bulk_size);
        client_out.err = EBUSY;
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                          &bulk_handle);
    }

    client_out.err = daemon_out.first;
    client_out.io_size = daemon_out.second;
    PROXY_DATA->log()->debug("{}() Sending output err '{}' io_size '{}'",
                             __func__, client_out.err, client_out.io_size);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                      &bulk_handle);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_read)

/**
 * RPC handler for an incoming write RPC
 * @param handle
 * @return
 */
static hg_return_t
proxy_rpc_srv_truncate(hg_handle_t handle) {
    rpc_client_proxy_trunc_in_t client_in{};
    rpc_err_out_t client_out{};

    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug(
            "{}() Got RPC with path '{}' current_size '{}' length '{}'",
            __func__, client_in.path, client_in.current_size, client_in.length);

    client_out.err = gkfs::rpc::forward_truncate(
            client_in.path, client_in.current_size, client_in.length);

    PROXY_DATA->log()->debug("{}() Sending output err '{}'", __func__,
                             client_out.err);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_truncate)

static hg_return_t
proxy_rpc_srv_chunk_stat(hg_handle_t handle) {
    rpc_chunk_stat_in_t client_in{};
    rpc_chunk_stat_out_t client_out{};
    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug("{}() Got chunk stat RPC ", __func__);

    auto daemon_out = gkfs::rpc::forward_get_chunk_stat();
    client_out.err = daemon_out.first;
    client_out.chunk_free = daemon_out.second.chunk_free;
    client_out.chunk_total = daemon_out.second.chunk_total;
    client_out.chunk_size = daemon_out.second.chunk_size;

    PROXY_DATA->log()->debug("{}() Sending output err '{}'", __func__,
                             client_out.err);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_chunk_stat)