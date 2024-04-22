/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <proxy/rpc/forward_metadata.hpp>
#include <common/rpc/distributor.hpp>
#include <common/rpc/rpc_types.hpp>

#include <sys/stat.h>

using namespace std;

namespace {

std::tuple<int, int64_t, uint32_t>
remove_metadata(const std::string& path) {
    hg_handle_t rpc_handle = nullptr;
    rpc_rm_node_in_t daemon_in{};
    rpc_rm_metadata_out_t daemon_out{};
    int err = 0;
    int64_t size = 0;
    uint32_t mode = 0;
    // fill in
    daemon_in.path = path.c_str();
    // Create handle
    PROXY_DATA->log()->debug("{}() Creating Margo handle ...", __func__);
    auto endp = PROXY_DATA->rpc_endpoints().at(
            PROXY_DATA->distributor()->locate_file_metadata(path, 0));
    auto ret = margo_create(PROXY_DATA->client_rpc_mid(), endp,
                            PROXY_DATA->rpc_client_ids().rpc_remove_id,
                            &rpc_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Critical error", __func__);
        return make_tuple(EBUSY, 0, 0);
        ;
    }
    ret = margo_forward(rpc_handle, &daemon_in);
    if(ret == HG_SUCCESS) {
        // Get response
        PROXY_DATA->log()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(rpc_handle, &daemon_out);
        if(ret == HG_SUCCESS) {
            PROXY_DATA->log()->debug("{}() Got response success: {}", __func__,
                                     daemon_out.err);
            mode = daemon_out.mode;
            size = daemon_out.size;
            err = daemon_out.err;
            margo_free_output(rpc_handle, &daemon_out);
        } else {
            // something is wrong
            err = EBUSY;
            PROXY_DATA->log()->error("{}() while getting rpc output", __func__);
        }
    } else {
        // something is wrong
        err = EBUSY;
        PROXY_DATA->log()->error("{}() Critical error", __func__);
    }

    /* clean up resources consumed by this rpc */
    margo_destroy(rpc_handle);
    return make_tuple(err, size, mode);
}

int
remove_data(const std::string& path) {
    int err = 0;
    // Create handles
    vector<hg_handle_t> rpc_handles(PROXY_DATA->hosts_size());
    vector<margo_request> rpc_waiters(PROXY_DATA->hosts_size());
    vector<rpc_rm_node_in_t> rpc_in(PROXY_DATA->hosts_size());
    for(size_t i = 0; i < PROXY_DATA->hosts_size(); i++) {
        rpc_in[i].path = path.c_str();
        PROXY_DATA->log()->trace(
                "{}() Sending non-blocking RPC to '{}': path '{}' ", __func__,
                i, rpc_in[i].path);
        auto ret = margo_create(PROXY_DATA->client_rpc_mid(),
                                PROXY_DATA->rpc_endpoints().at(i),
                                PROXY_DATA->rpc_client_ids().rpc_remove_data_id,
                                &rpc_handles[i]);
        if(ret != HG_SUCCESS) {
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            return EBUSY;
        }
        // Send RPC
        ret = margo_iforward(rpc_handles[i], &rpc_in[i], &rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to send non-blocking rpc for path {} and recipient {}",
                    __func__, path, i);
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            return EBUSY;
        }
    }
    PROXY_DATA->log()->debug("{}() '{}' RPCs sent, waiting for reply ...",
                             __func__, PROXY_DATA->hosts_size());
    // Wait for RPC responses and then get response
    for(uint64_t i = 0; i < PROXY_DATA->hosts_size(); i++) {
        auto ret = margo_wait(rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to wait for margo_request handle for path {} recipient {}",
                    __func__, path, i);
            err = EBUSY;
        }
        // decode response
        rpc_err_out_t out{};
        ret = margo_get_output(rpc_handles[i], &out);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Failed to get rpc output for path {} recipient {}",
                    __func__, path, i);
            err = EBUSY;
        }
        PROXY_DATA->log()->debug("{}() Got response from target '{}': err '{}'",
                                 __func__, i, out.err);
        if(out.err != 0)
            err = out.err;
        margo_free_output(rpc_handles[i], &out);
        margo_destroy(rpc_handles[i]);
    }
    return err;
}
} // namespace

namespace gkfs::rpc {

int
forward_create(const std::string& path, const mode_t mode) {
    hg_handle_t rpc_handle = nullptr;
    rpc_mk_node_in_t daemon_in{};
    rpc_err_out_t daemon_out{};
    int err = 0;
    // fill in
    daemon_in.path = path.c_str();
    daemon_in.mode = mode;
    // Create handle
    PROXY_DATA->log()->debug("{}() Creating Margo handle ...", __func__);
    auto endp = PROXY_DATA->rpc_endpoints().at(
            PROXY_DATA->distributor()->locate_file_metadata(path, 0));
    auto ret = margo_create(PROXY_DATA->client_rpc_mid(), endp,
                            PROXY_DATA->rpc_client_ids().rpc_create_id,
                            &rpc_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Critical error", __func__);
        return EBUSY;
    }
    ret = margo_forward(rpc_handle, &daemon_in);
    if(ret == HG_SUCCESS) {
        // Get response
        PROXY_DATA->log()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(rpc_handle, &daemon_out);
        if(ret == HG_SUCCESS) {
            PROXY_DATA->log()->debug("{}() Got response success: {}", __func__,
                                     daemon_out.err);
            err = daemon_out.err;
            margo_free_output(rpc_handle, &daemon_out);
        } else {
            // something is wrong
            err = EBUSY;
            PROXY_DATA->log()->error("{}() while getting rpc output", __func__);
        }
    } else {
        // something is wrong
        err = EBUSY;
        PROXY_DATA->log()->error("{}() sending rpc failed", __func__);
    }

    /* clean up resources consumed by this rpc */
    margo_destroy(rpc_handle);
    return err;
}

std::pair<int, std::string>
forward_stat(const std::string& path) {
    hg_handle_t rpc_handle = nullptr;
    rpc_path_only_in_t daemon_in{};
    rpc_stat_out_t daemon_out{};
    int err = 0;
    string attr{};
    // fill in
    daemon_in.path = path.c_str();
    // Create handle
    PROXY_DATA->log()->debug("{}() Creating Margo handle ...", __func__);
    auto endp = PROXY_DATA->rpc_endpoints().at(
            PROXY_DATA->distributor()->locate_file_metadata(path, 0));
    auto ret =
            margo_create(PROXY_DATA->client_rpc_mid(), endp,
                         PROXY_DATA->rpc_client_ids().rpc_stat_id, &rpc_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Critical error", __func__);
        return make_pair(EBUSY, attr);
    }
    ret = margo_forward(rpc_handle, &daemon_in);
    if(ret == HG_SUCCESS) {
        // Get response
        PROXY_DATA->log()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(rpc_handle, &daemon_out);
        if(ret == HG_SUCCESS) {
            PROXY_DATA->log()->debug("{}() Got response success: {}", __func__,
                                     daemon_out.err);
            err = daemon_out.err;
            if(err == 0)
                attr = daemon_out.db_val;
            margo_free_output(rpc_handle, &daemon_out);
        } else {
            // something is wrong
            err = EBUSY;
            PROXY_DATA->log()->error("{}() while getting rpc output", __func__);
        }
    } else {
        // something is wrong
        err = EBUSY;
        PROXY_DATA->log()->error("{}() sending rpc failed", __func__);
    }

    /* clean up resources consumed by this rpc */
    margo_destroy(rpc_handle);
    return make_pair(err, attr);
}

int
forward_remove(const std::string& path) {
    auto [err, mode, size] = remove_metadata(path);
    if(err != 0) {
        return err;
    }
    // if file is not a regular file and it's size is 0, data does not need to
    // be removed, thus, we exit
    if(!(S_ISREG(mode) && (size != 0))) {
        return 0;
    }
    return remove_data(path);
}

int
forward_decr_size(const std::string& path, size_t length) {
    hg_handle_t rpc_handle = nullptr;
    rpc_trunc_in_t daemon_in{};
    rpc_err_out_t daemon_out{};
    int err = 0;
    // fill in
    daemon_in.path = path.c_str();
    daemon_in.length = length;
    // Create handle
    PROXY_DATA->log()->debug("{}() Creating Margo handle ...", __func__);
    auto endp = PROXY_DATA->rpc_endpoints().at(
            PROXY_DATA->distributor()->locate_file_metadata(path, 0));
    auto ret = margo_create(PROXY_DATA->client_rpc_mid(), endp,
                            PROXY_DATA->rpc_client_ids().rpc_decr_size_id,
                            &rpc_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Critical error", __func__);
        return EBUSY;
    }
    ret = margo_forward(rpc_handle, &daemon_in);
    if(ret == HG_SUCCESS) {
        // Get response
        PROXY_DATA->log()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(rpc_handle, &daemon_out);
        if(ret == HG_SUCCESS) {
            PROXY_DATA->log()->debug("{}() Got response success: {}", __func__,
                                     daemon_out.err);
            err = daemon_out.err;
            margo_free_output(rpc_handle, &daemon_out);
        } else {
            // something is wrong
            err = EBUSY;
            PROXY_DATA->log()->error("{}() while getting rpc output", __func__);
        }
    } else {
        // something is wrong
        err = EBUSY;
        PROXY_DATA->log()->error("{}() sending rpc failed", __func__);
    }

    /* clean up resources consumed by this rpc */
    margo_destroy(rpc_handle);
    return err;
}

pair<int, off64_t>
forward_get_metadentry_size(const string& path) {
    hg_handle_t rpc_handle = nullptr;
    rpc_path_only_in_t daemon_in{};
    rpc_get_metadentry_size_out_t daemon_out{};
    int err = 0;
    off64_t ret_offset = 0;
    // fill in
    daemon_in.path = path.c_str();
    // Create handle
    PROXY_DATA->log()->debug("{}() Creating Margo handle ...", __func__);
    auto endp = PROXY_DATA->rpc_endpoints().at(
            PROXY_DATA->distributor()->locate_file_metadata(path, 0));
    auto ret = margo_create(
            PROXY_DATA->client_rpc_mid(), endp,
            PROXY_DATA->rpc_client_ids().rpc_get_metadentry_size_id,
            &rpc_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Critical error", __func__);
        return make_pair(EBUSY, 0);
        ;
    }
    ret = margo_forward(rpc_handle, &daemon_in);
    if(ret == HG_SUCCESS) {
        // Get response
        PROXY_DATA->log()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(rpc_handle, &daemon_out);
        if(ret == HG_SUCCESS) {
            PROXY_DATA->log()->debug(
                    "{}() Got response success err '{}' ret_size '{}'",
                    __func__, daemon_out.err, daemon_out.ret_size);
            err = daemon_out.err;
            ret_offset = daemon_out.ret_size;
            margo_free_output(rpc_handle, &daemon_out);
        } else {
            // something is wrong
            err = EBUSY;
            PROXY_DATA->log()->error("{}() while getting rpc output", __func__);
        }
    } else {
        // something is wrong
        err = EBUSY;
        PROXY_DATA->log()->error("{}() sending rpc failed", __func__);
    }

    /* clean up resources consumed by this rpc */
    margo_destroy(rpc_handle);
    return make_pair(err, ret_offset);
}

pair<int, off64_t>
forward_update_metadentry_size(const string& path, const size_t size,
                               const off64_t offset, const bool append_flag) {
    hg_handle_t rpc_handle = nullptr;
    rpc_update_metadentry_size_in_t daemon_in{};
    rpc_update_metadentry_size_out_t daemon_out{};
    int err = 0;
    off64_t ret_offset = 0;
    // fill in
    daemon_in.path = path.c_str();
    daemon_in.size = size;
    daemon_in.offset = offset;
    daemon_in.append = append_flag;
    // Create handle
    PROXY_DATA->log()->debug("{}() Creating Margo handle ...", __func__);
    auto endp = PROXY_DATA->rpc_endpoints().at(
            PROXY_DATA->distributor()->locate_file_metadata(path, 0));
    auto ret = margo_create(
            PROXY_DATA->client_rpc_mid(), endp,
            PROXY_DATA->rpc_client_ids().rpc_update_metadentry_size_id,
            &rpc_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Critical error", __func__);
        return make_pair(EBUSY, 0);
    }
    ret = margo_forward(rpc_handle, &daemon_in);
    if(ret == HG_SUCCESS) {
        // Get response
        PROXY_DATA->log()->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(rpc_handle, &daemon_out);
        if(ret == HG_SUCCESS) {
            PROXY_DATA->log()->debug(
                    "{}() Got response success: err {} ret_offset {}", __func__,
                    daemon_out.err, daemon_out.ret_offset);
            err = daemon_out.err;
            ret_offset = daemon_out.ret_offset;
            margo_free_output(rpc_handle, &daemon_out);
        } else {
            // something is wrong
            err = EBUSY;
            PROXY_DATA->log()->error("{}() while getting rpc output", __func__);
        }
    } else {
        // something is wrong
        err = EBUSY;
        PROXY_DATA->log()->error("{}() sending rpc failed", __func__);
    }

    /* clean up resources consumed by this rpc */
    margo_destroy(rpc_handle);
    return make_pair(err, ret_offset);
}

pair<int, size_t>
forward_get_dirents_single(const std::string& path, int server, void* buf,
                           size_t bulk_size) {
    hg_bulk_t bulk_handle = nullptr;
    hg_handle_t rpc_handle = nullptr;
    rpc_get_dirents_in_t daemon_in{};
    // register local target buffer for bulk access
    auto* bulk_buf = buf;
    auto size = make_shared<size_t>(bulk_size); // XXX Why shared ptr?
    auto ret = margo_bulk_create(PROXY_DATA->client_rpc_mid(), 1, &bulk_buf,
                                 size.get(), HG_BULK_WRITE_ONLY, &bulk_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to create rpc bulk handle",
                                 __func__);
        return ::make_pair(EBUSY, 0);
    }
    daemon_in.path = path.c_str();
    daemon_in.bulk_handle = bulk_handle;
    auto* endp = PROXY_DATA->rpc_endpoints().at(server);
    ret = margo_create(PROXY_DATA->client_rpc_mid(), endp,
                       PROXY_DATA->rpc_client_ids().rpc_get_dirents_extended_id,
                       &rpc_handle);
    if(ret != HG_SUCCESS) {
        margo_destroy(rpc_handle);
        margo_bulk_free(bulk_handle);
        return ::make_pair(EBUSY, 0);
    }
    // Send RPC
    margo_request rpc_waiter{};
    ret = margo_iforward(rpc_handle, &daemon_in, &rpc_waiter);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error(
                "{}() Unable to send non-blocking rpc for path {} and recipient {}",
                __func__, path, server);
        margo_destroy(rpc_handle);
        margo_bulk_free(bulk_handle);
        return ::make_pair(EBUSY, 0);
    }
    PROXY_DATA->log()->debug("{}() 1 RPC sent, waiting for reply ...",
                             __func__);
    int err = 0;
    size_t dirents_size = 0;
    ret = margo_wait(rpc_waiter);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error(
                "{}() Unable to wait for margo_request handle for path {} recipient {}",
                __func__, path, server);
        err = EBUSY;
    }
    // decode response
    rpc_get_dirents_out_t daemon_out{};
    ret = margo_get_output(rpc_handle, &daemon_out);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error(
                "{}() Failed to get rpc output for path {} recipient {}",
                __func__, path, server);
        err = EBUSY;
    }
    PROXY_DATA->log()->debug(
            "{}() Got response from target '{}': err '{}' with dirent_size '{}'",
            __func__, server, daemon_out.err, daemon_out.dirents_size);
    if(daemon_out.err != 0)
        err = daemon_out.err;
    else
        dirents_size = daemon_out.dirents_size;
    margo_free_output(rpc_handle, &daemon_out);
    margo_destroy(rpc_handle);
    margo_bulk_free(bulk_handle);
    return ::make_pair(err, dirents_size);
}

} // namespace gkfs::rpc
