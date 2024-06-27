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
#include <proxy/rpc/forward_metadata.hpp>
#include <proxy/rpc/rpc_util.hpp>

#include <common/rpc/rpc_types.hpp>

static hg_return_t
proxy_rpc_srv_create(hg_handle_t handle) {
    rpc_mk_node_in_t client_in{};
    rpc_err_out_t client_out{};

    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug("{}() Got RPC with path '{}'", __func__,
                             client_in.path);

    client_out.err = gkfs::rpc::forward_create(client_in.path, client_in.mode);

    PROXY_DATA->log()->debug("{}() Sending output err '{}'", __func__,
                             client_out.err);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_create)

static hg_return_t
proxy_rpc_srv_stat(hg_handle_t handle) {
    rpc_path_only_in_t client_in{};
    rpc_stat_out_t client_out{};

    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug("{}() Got RPC with path '{}'", __func__,
                             client_in.path);

    auto out = gkfs::rpc::forward_stat(client_in.path);
    client_out.err = out.first;
    client_out.db_val = out.second.c_str();

    PROXY_DATA->log()->debug("{}() Sending output err '{}'", __func__,
                             client_out.err);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_stat)

static hg_return_t
proxy_rpc_srv_remove(hg_handle_t handle) {
    rpc_rm_node_in_t client_in{};
    rpc_err_out_t client_out{};

    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug("{}() Got RPC with path '{}'", __func__,
                             client_in.path);
    client_out.err = gkfs::rpc::forward_remove(client_in.path);

    PROXY_DATA->log()->debug("{}() Sending output err '{}'", __func__,
                             client_out.err);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_remove)

static hg_return_t
proxy_rpc_srv_decr_size(hg_handle_t handle) {
    rpc_trunc_in_t client_in{};
    rpc_err_out_t client_out{};

    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug("{}() Got RPC with path '{}' length '{}'",
                             __func__, client_in.path, client_in.length);
    client_out.err =
            gkfs::rpc::forward_decr_size(client_in.path, client_in.length);

    PROXY_DATA->log()->debug("{}() Sending output err '{}'", __func__,
                             client_out.err);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_decr_size)

static hg_return_t
proxy_rpc_srv_get_metadentry_size(hg_handle_t handle) {

    rpc_path_only_in_t client_in{};
    rpc_get_metadentry_size_out_t client_out{};

    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug("{}() path: '{}'", __func__, client_in.path);

    try {
        auto [err, ret_size] =
                gkfs::rpc::forward_get_metadentry_size(client_in.path);
        client_out.err = 0;
        client_out.ret_size = ret_size;
    } catch(const std::exception& e) {
        PROXY_DATA->log()->error("{}() Failed to get metadentry size RPC: '{}'",
                                 __func__, e.what());
        client_out.err = EBUSY;
    }

    PROXY_DATA->log()->debug("{}() Sending output err '{}' ret_size '{}'",
                             __func__, client_out.err, client_out.ret_size);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_get_metadentry_size)

static hg_return_t
proxy_rpc_srv_update_metadentry_size(hg_handle_t handle) {

    rpc_update_metadentry_size_in_t client_in{};
    rpc_update_metadentry_size_out_t client_out{};


    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug(
            "{}() path: '{}', size: '{}', offset: '{}', append: '{}'", __func__,
            client_in.path, client_in.size, client_in.offset, client_in.append);

    try {
        auto [err, ret_offset] = gkfs::rpc::forward_update_metadentry_size(
                client_in.path, client_in.size, client_in.offset,
                client_in.append);

        client_out.err = 0;
        client_out.ret_offset = ret_offset;
    } catch(const std::exception& e) {
        PROXY_DATA->log()->error(
                "{}() Failed to update metadentry size RPC: '{}'", __func__,
                e.what());
        client_out.err = EBUSY;
    }

    PROXY_DATA->log()->debug("{}() Sending output err '{}' ret_offset '{}'",
                             __func__, client_out.err, client_out.ret_offset);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_update_metadentry_size)

static hg_return_t
proxy_rpc_srv_get_dirents_extended(hg_handle_t handle) {

    rpc_proxy_get_dirents_in_t client_in{};
    rpc_get_dirents_out_t client_out{};
    hg_bulk_t bulk_handle = nullptr;

    auto ret = margo_get_input(handle, &client_in);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to retrieve input from handle",
                                 __func__);
        return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out);
    }
    PROXY_DATA->log()->debug("{}() path: '{}', server: '{}'", __func__,
                             client_in.path, client_in.server);

    auto hgi = margo_get_info(handle);
    auto mid = margo_hg_handle_get_instance(handle);
    auto bulk_size = margo_bulk_get_size(client_in.bulk_handle);
    PROXY_DATA->log()->debug("{}() Got RPC with path '{}' bulk_size '{}'",
                             __func__, client_in.path, bulk_size);
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
    auto daemon_out = gkfs::rpc::forward_get_dirents_single(
            client_in.path, client_in.server, bulk_buf, bulk_size);
    if(daemon_out.first != 0) {
        PROXY_DATA->log()->error(
                "{}() Failure when forwarding to daemon with err '{}'",
                __func__, daemon_out.first);
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
    client_out.dirents_size = daemon_out.second;
    PROXY_DATA->log()->debug("{}() Sending output err '{}' dirents_size '{}'",
                             __func__, client_out.err, client_out.dirents_size);
    return gkfs::rpc::cleanup_respond(&handle, &client_in, &client_out,
                                      &bulk_handle);
}

DEFINE_MARGO_RPC_HANDLER(proxy_rpc_srv_get_dirents_extended)
