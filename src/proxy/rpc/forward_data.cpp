/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <proxy/rpc/forward_data.hpp>

#include <common/rpc/rpc_types.hpp>
#include <common/arithmetic/arithmetic.hpp>
#include <common/rpc/distributor.hpp>

#include <map>
#include <unordered_set>

using namespace std;

namespace gkfs::rpc {

std::pair<int, ssize_t>
forward_write(const std::string& path, void* buf, const int64_t offset,
              const size_t write_size) {
    // import pow2-optimized arithmetic functions
    using namespace gkfs::utils::arithmetic;
    // TODO mostly copy pasta from forward_data on client w.r.t. chunking logic
    // (actually old margo code pre-hermes)
    hg_bulk_t bulk_handle = nullptr;
    // register local target buffer for bulk access
    auto bulk_buf = buf;
    auto size = make_shared<size_t>(write_size); // XXX Why shared ptr?
    auto ret = margo_bulk_create(PROXY_DATA->client_rpc_mid(), 1, &bulk_buf,
                                 size.get(), HG_BULK_READ_ONLY, &bulk_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to create rpc bulk handle",
                                 __func__);
        return ::make_pair(EBUSY, 0);
    }
    auto chnk_start = block_index(offset, gkfs::config::rpc::chunksize);
    auto chnk_end = block_index((offset + write_size) - 1,
                                gkfs::config::rpc::chunksize);

    // Collect all chunk ids within count that have the same destination so
    // that those are send in one rpc bulk transfer
    ::map<uint64_t, std::vector<uint64_t>> target_chnks{};
    // contains the target ids, used to access the target_chnks map.
    // First idx is chunk with potential offset
    ::vector<uint64_t> targets{};

    // targets for the first and last chunk as they need special treatment
    uint64_t chnk_start_target = 0;
    uint64_t chnk_end_target = 0;

    for(uint64_t chnk_id = chnk_start; chnk_id <= chnk_end; chnk_id++) {
        auto target = PROXY_DATA->distributor()->locate_data(path, chnk_id, 0);

        if(target_chnks.count(target) == 0) {
            target_chnks.insert(
                    std::make_pair(target, std::vector<uint64_t>{chnk_id}));
            targets.push_back(target);
        } else {
            target_chnks[target].push_back(chnk_id);
        }

        // set first and last chnk targets
        if(chnk_id == chnk_start) {
            chnk_start_target = target;
        }

        if(chnk_id == chnk_end) {
            chnk_end_target = target;
        }
    }
    // some helper variables for async RPC
    auto target_n = targets.size();
    ::vector<hg_handle_t> rpc_handles(target_n);
    ::vector<margo_request> rpc_waiters(target_n);
    ::vector<rpc_proxy_daemon_write_in_t> rpc_in(target_n);
    // Issue non-blocking RPC requests and wait for the result later
    for(uint64_t i = 0; i < target_n; i++) {
        auto target = targets[i];
        auto total_chunk_size =
                target_chnks[target].size() *
                gkfs::config::rpc::chunksize; // total chunk_size for target
        if(target == chnk_start_target) // receiver of first chunk must subtract
                                        // the offset from first chunk
            total_chunk_size -=
                    block_overrun(offset, gkfs::config::rpc::chunksize);
        // receiver of last chunk must subtract
        if(target == chnk_end_target &&
           !is_aligned(offset + write_size, gkfs::config::rpc::chunksize))
            total_chunk_size -= block_underrun(offset + write_size,
                                               gkfs::config::rpc::chunksize);
        // Fill RPC input
        rpc_in[i].path = path.c_str();
        rpc_in[i].offset = block_overrun(
                offset,
                gkfs::config::rpc::chunksize); // first offset in targets is the
                                               // chunk with a potential offset
        rpc_in[i].host_id = target;
        rpc_in[i].host_size = PROXY_DATA->rpc_endpoints().size();
        rpc_in[i].chunk_n =
                target_chnks[target]
                        .size(); // number of chunks handled by that destination
        rpc_in[i].chunk_start = chnk_start; // chunk start id of this write
        rpc_in[i].chunk_end = chnk_end;     // chunk end id of this write
        rpc_in[i].total_chunk_size = total_chunk_size; // total size to write
        rpc_in[i].bulk_handle = bulk_handle;
        PROXY_DATA->log()->trace(
                "{}() Sending non-blocking RPC to '{}': path '{}' offset '{}' chunk_n '{}' chunk_start '{}' chunk_end '{}' total_chunk_size '{}'",
                __func__, target, rpc_in[i].path, rpc_in[i].offset,
                rpc_in[i].chunk_n, rpc_in[i].chunk_start, rpc_in[i].chunk_end,
                rpc_in[i].total_chunk_size);
        ret = margo_create(PROXY_DATA->client_rpc_mid(),
                           PROXY_DATA->rpc_endpoints().at(target),
                           PROXY_DATA->rpc_client_ids().rpc_write_id,
                           &rpc_handles[i]);
        if(ret != HG_SUCCESS) {
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            margo_bulk_free(bulk_handle);
            return ::make_pair(EBUSY, 0);
        }
        // Send RPC
        ret = margo_iforward(rpc_handles[i], &rpc_in[i], &rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to send non-blocking rpc for path {} and recipient {}",
                    __func__, path, target);
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            margo_bulk_free(bulk_handle);
            return ::make_pair(EBUSY, 0);
        }
    }
    PROXY_DATA->log()->debug("{}() '{}' RPCs sent, waiting for reply ...",
                             __func__, target_n);
    // Wait for RPC responses and then get response and add it to out_size which
    // is the written size All potential outputs are served to free resources
    // regardless of errors, although an errorcode is set.
    ssize_t out_size = 0;
    int err = 0;
    for(uint64_t i = 0; i < target_n; i++) {
        // XXX We might need a timeout here to not wait forever for an output
        // that never comes?
        ret = margo_wait(rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to wait for margo_request handle for path {} recipient {}",
                    __func__, path, targets[i]);
            err = EBUSY;
        }
        // decode response
        rpc_data_out_t out{};
        ret = margo_get_output(rpc_handles[i], &out);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Failed to get rpc output for path {} recipient {}",
                    __func__, path, targets[i]);
            err = EBUSY;
        }
        PROXY_DATA->log()->debug(
                "{}() Got response from target '{}': err '{}' with io_size '{}'",
                __func__, i, out.err, out.io_size);
        if(out.err != 0)
            err = out.err;
        else
            out_size += static_cast<size_t>(out.io_size);
        margo_free_output(rpc_handles[i], &out);
        margo_destroy(rpc_handles[i]);
    }
    margo_bulk_free(bulk_handle);
    return ::make_pair(err, out_size);
}

std::pair<int, ssize_t>
forward_read(const std::string& path, void* buf, const int64_t offset,
             const size_t read_size) {
    // import pow2-optimized arithmetic functions
    using namespace gkfs::utils::arithmetic;
    // TODO mostly copy pasta from forward_data on client w.r.t. chunking logic
    // (actually old margo code pre-hermes)
    hg_bulk_t bulk_handle = nullptr;
    // register local target buffer for bulk access
    auto bulk_buf = buf;
    auto size = make_shared<size_t>(read_size); // XXX Why shared ptr?
    auto ret = margo_bulk_create(PROXY_DATA->client_rpc_mid(), 1, &bulk_buf,
                                 size.get(), HG_BULK_WRITE_ONLY, &bulk_handle);
    if(ret != HG_SUCCESS) {
        PROXY_DATA->log()->error("{}() Failed to create rpc bulk handle",
                                 __func__);
        return ::make_pair(EBUSY, 0);
    }

    // Calculate chunkid boundaries and numbers so that daemons know in which
    // interval to look for chunks
    auto chnk_start = block_index(offset, gkfs::config::rpc::chunksize);
    auto chnk_end =
            block_index((offset + read_size - 1), gkfs::config::rpc::chunksize);

    // Collect all chunk ids within count that have the same destination so
    // that those are send in one rpc bulk transfer
    std::map<uint64_t, std::vector<uint64_t>> target_chnks{};
    // contains the recipient ids, used to access the target_chnks map.
    // First idx is chunk with potential offset
    std::vector<uint64_t> targets{};

    // targets for the first and last chunk as they need special treatment
    uint64_t chnk_start_target = 0;
    uint64_t chnk_end_target = 0;

    for(uint64_t chnk_id = chnk_start; chnk_id <= chnk_end; chnk_id++) {
        auto target = PROXY_DATA->distributor()->locate_data(path, chnk_id, 0);

        if(target_chnks.count(target) == 0) {
            target_chnks.insert(
                    std::make_pair(target, std::vector<uint64_t>{chnk_id}));
            targets.push_back(target);
        } else {
            target_chnks[target].push_back(chnk_id);
        }

        // set first and last chnk targets
        if(chnk_id == chnk_start) {
            chnk_start_target = target;
        }

        if(chnk_id == chnk_end) {
            chnk_end_target = target;
        }
    }

    // some helper variables for async RPC
    auto target_n = targets.size();
    vector<hg_handle_t> rpc_handles(target_n);
    vector<margo_request> rpc_waiters(target_n);
    vector<rpc_proxy_daemon_read_in_t> rpc_in(target_n);
    // Issue non-blocking RPC requests and wait for the result later
    for(uint64_t i = 0; i < target_n; i++) {
        auto target = targets[i];
        auto total_chunk_size =
                target_chnks[target].size() * gkfs::config::rpc::chunksize;
        if(target == chnk_start_target) // receiver of first chunk must subtract
                                        // the offset from first chunk
            total_chunk_size -=
                    block_overrun(offset, gkfs::config::rpc::chunksize);
        // receiver of last chunk must subtract
        if(target == chnk_end_target &&
           !is_aligned(offset + read_size, gkfs::config::rpc::chunksize))
            total_chunk_size -= block_underrun(offset + read_size,
                                               gkfs::config::rpc::chunksize);

        // Fill RPC input
        rpc_in[i].path = path.c_str();
        rpc_in[i].offset = block_overrun(
                offset,
                gkfs::config::rpc::chunksize); // first offset in targets is the
                                               // chunk with a potential offset
        rpc_in[i].host_id = target;
        rpc_in[i].host_size = PROXY_DATA->rpc_endpoints().size();
        rpc_in[i].chunk_n =
                target_chnks[target]
                        .size(); // number of chunks handled by that destination
        rpc_in[i].chunk_start = chnk_start; // chunk start id of this write
        rpc_in[i].chunk_end = chnk_end;     // chunk end id of this write
        rpc_in[i].total_chunk_size = total_chunk_size; // total size to write
        rpc_in[i].bulk_handle = bulk_handle;
        PROXY_DATA->log()->trace(
                "{}() Sending non-blocking RPC to '{}': path '{}' offset '{}' chunk_n '{}' chunk_start '{}' chunk_end '{}' total_chunk_size '{}'",
                __func__, target, rpc_in[i].path, rpc_in[i].offset,
                rpc_in[i].chunk_n, rpc_in[i].chunk_start, rpc_in[i].chunk_end,
                rpc_in[i].total_chunk_size);

        ret = margo_create(PROXY_DATA->client_rpc_mid(),
                           PROXY_DATA->rpc_endpoints().at(target),
                           PROXY_DATA->rpc_client_ids().rpc_read_id,
                           &rpc_handles[i]);
        if(ret != HG_SUCCESS) {
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            margo_bulk_free(bulk_handle);
            return ::make_pair(EBUSY, 0);
        }
        // Send RPC
        ret = margo_iforward(rpc_handles[i], &rpc_in[i], &rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to send non-blocking rpc for path {} and recipient {}",
                    __func__, path, target);
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            margo_bulk_free(bulk_handle);
            return ::make_pair(EBUSY, 0);
        }
    }

    PROXY_DATA->log()->debug("{}() '{}' RPCs sent, waiting for reply ...",
                             __func__, target_n);
    // Wait for RPC responses and then get response and add it to out_size which
    // is the written size All potential outputs are served to free resources
    // regardless of errors, although an errorcode is set.
    ssize_t out_size = 0;
    int err = 0;
    for(uint64_t i = 0; i < target_n; i++) {
        // XXX We might need a timeout here to not wait forever for an output
        // that never comes?
        ret = margo_wait(rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to wait for margo_request handle for path {} recipient {}",
                    __func__, path, targets[i]);
            err = EBUSY;
        }
        // decode response
        rpc_data_out_t out{};
        ret = margo_get_output(rpc_handles[i], &out);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Failed to get rpc output for path {} recipient {}",
                    __func__, path, targets[i]);
            err = EBUSY;
        }
        PROXY_DATA->log()->debug(
                "{}() Got response from target '{}': err '{}' with io_size '{}'",
                __func__, i, out.err, out.io_size);
        if(out.err != 0)
            err = out.err;
        else
            out_size += static_cast<size_t>(out.io_size);
        margo_free_output(rpc_handles[i], &out);
        margo_destroy(rpc_handles[i]);
    }
    margo_bulk_free(bulk_handle);
    return ::make_pair(err, out_size);
}

int
forward_truncate(const std::string& path, size_t current_size,
                 size_t new_size) {

    rpc_trunc_in_t daemon_in{};
    rpc_err_out_t daemon_out{};
    hg_return_t ret{};
    bool err = false;
    // fill in
    daemon_in.path = path.c_str();
    daemon_in.length = new_size;

    // import pow2-optimized arithmetic functions
    using namespace gkfs::utils::arithmetic;

    // Find out which data servers need to delete data chunks in order to
    // contact only them
    const unsigned int chunk_start =
            block_index(new_size, gkfs::config::rpc::chunksize);
    const unsigned int chunk_end = block_index(current_size - new_size - 1,
                                               gkfs::config::rpc::chunksize);

    std::unordered_set<unsigned int> hosts;
    for(unsigned int chunk_id = chunk_start; chunk_id <= chunk_end;
        ++chunk_id) {
        hosts.insert(PROXY_DATA->distributor()->locate_data(path, chunk_id, 0));
    }
    // some helper variables for async RPC
    vector<hg_handle_t> rpc_handles(hosts.size());
    vector<margo_request> rpc_waiters(hosts.size());
    unsigned int req_num = 0;
    // Issue non-blocking RPC requests and wait for the result later
    for(const auto& host : hosts) {

        ret = margo_create(PROXY_DATA->client_rpc_mid(),
                           PROXY_DATA->rpc_endpoints().at(host),
                           PROXY_DATA->rpc_client_ids().rpc_truncate_id,
                           &rpc_handles[req_num]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to create Mercury handle for host: ", __func__,
                    host);
            break;
        }
        // Send RPC
        ret = margo_iforward(rpc_handles[req_num], &daemon_in,
                             &rpc_waiters[req_num]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to send non-blocking rpc for path {} and recipient {}",
                    __func__, path, host);
            break;
        }
        req_num++;
    }
    if(req_num < hosts.size()) {
        // An error occurred. Cleanup and return
        PROXY_DATA->log()->error(
                "{}() Error -> sent only some requests {}/{}. Cancelling request...",
                __func__, req_num, hosts.size());
        for(unsigned int i = 0; i < req_num; ++i) {
            margo_destroy(rpc_handles[i]);
        }
        // TODO Ideally wait for dangling responses
        return EIO;
    }
    // Wait for RPC responses and then get response
    for(unsigned int i = 0; i < hosts.size(); ++i) {
        ret = margo_wait(rpc_waiters[i]);
        if(ret == HG_SUCCESS) {
            ret = margo_get_output(rpc_handles[i], &daemon_out);
            if(ret == HG_SUCCESS) {
                if(daemon_out.err) {
                    PROXY_DATA->log()->error("{}() received error response: {}",
                                             __func__, daemon_out.err);
                    err = true;
                }
            } else {
                // Get output failed
                PROXY_DATA->log()->error("{}() while getting rpc output",
                                         __func__);
                err = true;
            }
        } else {
            // Wait failed
            PROXY_DATA->log()->error("{}() Failed while waiting for response",
                                     __func__);
            err = true;
        }

        /* clean up resources consumed by this rpc */
        margo_free_output(rpc_handles[i], &daemon_out);
        margo_destroy(rpc_handles[i]);
    }

    if(err) {
        errno = EBUSY;
        return -1;
    }
    return 0;
}

pair<int, ChunkStat>
forward_get_chunk_stat() {
    int err = 0;
    hg_return ret{};
    // Create handle
    PROXY_DATA->log()->debug("{}() Creating Margo handle ...", __func__);

    // some helper variables for async RPC
    auto target_n = PROXY_DATA->hosts_size();
    vector<hg_handle_t> rpc_handles(target_n);
    vector<margo_request> rpc_waiters(target_n);
    vector<rpc_chunk_stat_in_t> rpc_in(target_n);
    for(uint64_t i = 0; i < target_n; i++) {
        ret = margo_create(PROXY_DATA->client_rpc_mid(),
                           PROXY_DATA->rpc_endpoints().at(i),
                           PROXY_DATA->rpc_client_ids().rpc_chunk_stat_id,
                           &rpc_handles[i]);
        // XXX Don't think this is useful here cause responds go into nothing
        if(ret != HG_SUCCESS) {
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            return ::make_pair(EBUSY, ChunkStat{});
        }
        // Send RPC
        rpc_in[i].dummy = 0;
        ret = margo_iforward(rpc_handles[i], &rpc_in[i], &rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to send non-blocking rpc for recipient {}",
                    __func__, i);
            for(uint64_t j = 0; j < i + 1; j++) {
                margo_destroy(rpc_handles[j]);
            }
            return ::make_pair(EBUSY, ChunkStat{});
        }
    }
    PROXY_DATA->log()->debug("{}() '{}' RPCs sent, waiting for reply ...",
                             __func__, target_n);
    // Wait for RPC responses and then get response and add it to out_size which
    // is the written size All potential outputs are served to free resources
    // regardless of errors, although an errorcode is set.
    unsigned long chunk_size = gkfs::config::rpc::chunksize;
    unsigned long chunk_total = 0;
    unsigned long chunk_free = 0;
    for(uint64_t i = 0; i < target_n; i++) {
        // XXX We might need a timeout here to not wait forever for an output
        // that never comes?
        ret = margo_wait(rpc_waiters[i]);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Unable to wait for margo_request handle for recipient {}",
                    __func__, i);
            err = EBUSY;
        }
        // decode response
        rpc_chunk_stat_out_t daemon_out{};
        ret = margo_get_output(rpc_handles[i], &daemon_out);
        if(ret != HG_SUCCESS) {
            PROXY_DATA->log()->error(
                    "{}() Failed to get rpc output for recipient {}", __func__,
                    i);
            err = EBUSY;
        }
        PROXY_DATA->log()->debug(
                "{}() Got response from target '{}': err '{}' with chunk_total '{}' chunk_free '{}'",
                __func__, i, daemon_out.err, daemon_out.chunk_total,
                daemon_out.chunk_free);
        if(daemon_out.err != 0)
            err = daemon_out.err;
        else {
            chunk_total += daemon_out.chunk_total;
            chunk_free += daemon_out.chunk_free;
        }
        margo_free_output(rpc_handles[i], &daemon_out);
        margo_destroy(rpc_handles[i]);
    }
    if(err)
        return make_pair(err, ChunkStat{});
    else
        return make_pair(0, ChunkStat{chunk_size, chunk_total, chunk_free});
}

} // namespace gkfs::rpc