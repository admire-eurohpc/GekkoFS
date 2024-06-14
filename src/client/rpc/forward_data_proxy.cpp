/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <client/preload_util.hpp>
#include <client/rpc/forward_data_proxy.hpp>
#include <client/rpc/rpc_types.hpp>
#include <client/logging.hpp>

#include <common/rpc/distributor.hpp>
#include <common/arithmetic/arithmetic.hpp>

#include <unordered_set>

using namespace std;

namespace gkfs::rpc {

/**
 * Actual chunking logic on proxy handler
 * @param path
 * @param buf
 * @param append_flag
 * @param offset
 * @param write_size
 * @param updated_metadentry_size
 * @return
 */
pair<int, ssize_t>
forward_write_proxy(const string& path, const void* buf, off64_t offset,
                    size_t write_size) {
    LOG(DEBUG, "Using write proxy for path '{}' offset '{}' size '{}' ...",
        path, offset, write_size);
    // TODO mostly copy pasta from forward_data.
    assert(write_size > 0);

    // Calculate chunkid boundaries and numbers so that daemons know in
    // which interval to look for chunks

    // some helper variables for async RPC
    std::vector<hermes::mutable_buffer> bufseq{
            hermes::mutable_buffer{const_cast<void*>(buf), write_size},
    };

    // expose user buffers so that they can serve as RDMA data sources
    // (these are automatically "unexposed" when the destructor is called)
    hermes::exposed_memory local_buffers;

    try {
        local_buffers = ld_proxy_service->expose(
                bufseq, hermes::access_mode::read_only);

    } catch(const std::exception& ex) {
        LOG(ERROR, "Failed to expose buffers for RMA");
        return make_pair(EBUSY, 0);
    }

    auto endp = CTX->proxy_host();
    auto err = 0;
    ssize_t out_size = 0;
    try {
        LOG(DEBUG, "Sending RPC ...");

        gkfs::rpc::write_data_proxy::input in(path, offset, write_size,
                                              local_buffers);
        LOG(DEBUG, "proxy-host: {}, path: '{}', size: {}, offset: {}", endp.to_string(),
            path, in.write_size(), in.offset());

        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that
        // we can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out = ld_proxy_service->post<gkfs::rpc::write_data_proxy>(endp, in)
                           .get()
                           .at(0);

        if(out.err()) {
            LOG(ERROR, "Daemon reported error: {}", out.err());
            err = out.err();
        }
        out_size = out.io_size();

    } catch(const std::exception& ex) {
        LOG(ERROR, "While RPC send or getting RPC output. Err: '{}'",
            ex.what());
        err = EBUSY;
    }
    if(err)
        return make_pair(err, 0);
    else
        return make_pair(0, out_size);
}

pair<int, ssize_t>
forward_read_proxy(const string& path, void* buf, const off64_t offset,
                   const size_t read_size) {
    LOG(DEBUG, "Using read proxy for path '{}' offset '{}' size '{}' ...", path,
        offset, read_size);

    // some helper variables for async RPCs
    std::vector<hermes::mutable_buffer> bufseq{
            hermes::mutable_buffer{buf, read_size},
    };

    // expose user buffers so that they can serve as RDMA data targets
    // (these are automatically "unexposed" when the destructor is called)
    hermes::exposed_memory local_buffers;

    try {
        local_buffers = ld_proxy_service->expose(
                bufseq, hermes::access_mode::write_only);

    } catch(const std::exception& ex) {
        LOG(ERROR, "Failed to expose buffers for RMA");
        errno = EBUSY;
        return make_pair(EBUSY, 0);
    }

    auto endp = CTX->proxy_host();
    auto err = 0;
    ssize_t out_size = 0;

    try {
        LOG(DEBUG, "Sending RPC ...");

        gkfs::rpc::read_data_proxy::input in(path, offset, read_size,
                                             local_buffers);
        LOG(DEBUG, "proxy-host: {}, path: '{}', size: {}, offset: {}", endp.to_string(),
            path, in.read_size(), in.offset());

        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that
        // we can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out = ld_proxy_service->post<gkfs::rpc::read_data_proxy>(endp, in)
                           .get()
                           .at(0);

        if(out.err()) {
            LOG(ERROR, "Daemon reported error: {}", out.err());
            err = out.err();
        }
        out_size = out.io_size();

    } catch(const std::exception& ex) {
        LOG(ERROR, "While RPC send or getting RPC output. Err: '{}'",
            ex.what());
        err = EBUSY;
    }

    if(err)
        return make_pair(err, 0);
    else
        return make_pair(0, out_size);
}

pair<int, ChunkStat>
forward_get_chunk_stat_proxy() {
    auto endp = CTX->proxy_host();
    gkfs::rpc::chunk_stat_proxy::input in(0);

    try {
        LOG(DEBUG, "Sending RPC ...");
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we
        // can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/

        auto out = ld_proxy_service->post<gkfs::rpc::chunk_stat_proxy>(endp, in)
                           .get()
                           .at(0);
        LOG(DEBUG, "Got response success: {}", out.err());

        if(out.err())
            return make_pair(out.err(), ChunkStat{});
        else
            return make_pair(0, ChunkStat{out.chunk_size(), out.chunk_total(),
                                          out.chunk_free()});
    } catch(const std::exception& ex) {
        LOG(ERROR, "while getting rpc output");
        return make_pair(EBUSY, ChunkStat{});
    }
}

} // namespace gkfs::rpc