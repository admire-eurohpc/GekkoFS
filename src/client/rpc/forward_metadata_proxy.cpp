/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <client/preload_util.hpp>
#include <client/rpc/forward_metadata_proxy.hpp>
#include <client/rpc/rpc_types.hpp>
#include <client/logging.hpp>

#include <common/rpc/rpc_util.hpp>

using namespace std;

namespace gkfs {
namespace rpc {

int
forward_create_proxy(const std::string& path, const mode_t mode) {
    auto endp = CTX->proxy_host();

    try {
        LOG(DEBUG, "{}() Sending RPC for path '{}'...", __func__, path);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we
        // can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out = ld_proxy_service
                           ->post<gkfs::rpc::create_proxy>(endp, path, mode)
                           .get()
                           .at(0);
        LOG(DEBUG, "Got response success: {}", out.err());

        return out.err() ? out.err() : 0;
    } catch(const std::exception& ex) {
        LOG(ERROR, "{}() getting rpc output for path '{}' failed", __func__,
            path);
        return EBUSY;
    }
}

int
forward_stat_proxy(const std::string& path, string& attr) {

    auto endp = CTX->proxy_host();

    try {
        LOG(DEBUG, "{}() Sending RPC for path '{}'...", __func__, path);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we
        // can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out = ld_proxy_service->post<gkfs::rpc::stat_proxy>(endp, path)
                           .get()
                           .at(0);
        LOG(DEBUG, "Got response success: {}", out.err());

        if(out.err())
            return out.err();

        attr = out.db_val();
        return 0;
    } catch(const std::exception& ex) {
        LOG(ERROR, "{}() getting rpc output for path '{}' failed", __func__,
            path);
        return EBUSY;
    }
}

int
forward_remove_proxy(const std::string& path) {
    auto endp = CTX->proxy_host();

    try {
        LOG(DEBUG, "{}() Sending RPC for path '{}'...", __func__, path);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we
        // can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out = ld_proxy_service->post<gkfs::rpc::remove_proxy>(endp, path)
                           .get()
                           .at(0);
        LOG(DEBUG, "Got response success: {}", out.err());

        return out.err() ? out.err() : 0;
    } catch(const std::exception& ex) {
        LOG(ERROR, "{}() getting rpc output for path '{}' failed", __func__,
            path);
        return EBUSY;
    }
}

int
forward_decr_size_proxy(const std::string& path, size_t length) {
    auto endp = CTX->proxy_host();

    try {
        LOG(DEBUG, "{}() Sending RPC for path '{}'...", __func__, path);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we
        // can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out =
                ld_proxy_service
                        ->post<gkfs::rpc::decr_size_proxy>(endp, path, length)
                        .get()
                        .at(0);
        LOG(DEBUG, "Got response success: {}", out.err());

        return out.err() ? out.err() : 0;
    } catch(const std::exception& ex) {
        LOG(ERROR, "{}() getting rpc output for path '{}' failed", __func__,
            path);
        return EBUSY;
    }
}

pair<int, off64_t>
forward_update_metadentry_size_proxy(const string& path, const size_t size,
                                     const off64_t offset,
                                     const bool append_flag) {

    auto endp = CTX->proxy_host();
    try {
        LOG(DEBUG, "Sending update size proxy RPC ...");
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we
        // can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out = ld_proxy_service
                           ->post<gkfs::rpc::update_metadentry_size_proxy>(
                                   endp, path, size, offset,
                                   bool_to_merc_bool(append_flag))
                           .get()
                           .at(0);

        LOG(DEBUG, "Got response success: {}", out.err());

        if(out.err())
            return make_pair(out.err(), 0);
        else
            return make_pair(0, out.ret_size());
    } catch(const std::exception& ex) {
        LOG(ERROR, "{}() getting rpc output for path '{}' failed", __func__,
            path);
        return make_pair(EBUSY, 0);
    }
}

pair<int, off64_t>
forward_get_metadentry_size_proxy(const std::string& path) {
    auto endp = CTX->proxy_host();

    try {
        LOG(DEBUG, "{}() Sending RPC for path '{}'...", __func__, path);
        // TODO(amiranda): add a post() with RPC_TIMEOUT to hermes so that we
        // can retry for RPC_TRIES (see old commits with margo)
        // TODO(amiranda): hermes will eventually provide a post(endpoint)
        // returning one result and a broadcast(endpoint_set) returning a
        // result_set. When that happens we can remove the .at(0) :/
        auto out =
                ld_proxy_service
                        ->post<gkfs::rpc::get_metadentry_size_proxy>(endp, path)
                        .get()
                        .at(0);

        LOG(DEBUG, "Got response success: {}", out.err());

        if(out.err())
            return make_pair(out.err(), 0);
        else
            return make_pair(0, out.ret_size());
    } catch(const std::exception& ex) {
        LOG(ERROR, "{}() getting rpc output for path '{}' failed", __func__,
            path);
        return make_pair(EBUSY, 0);
    }
}

pair<int, unique_ptr<vector<tuple<const std::string, bool, size_t, time_t>>>>
forward_get_dirents_single_proxy(const string& path, int server) {

    LOG(DEBUG, "{}() enter for path '{}'", __func__, path)
    auto endp = CTX->proxy_host();

    /* preallocate receiving buffer. The actual size is not known yet.
     *
     * On C++14 make_unique function also zeroes the newly allocated buffer.
     * It turns out that this operation is increadibly slow for such a big
     * buffer. Moreover we don't need a zeroed buffer here.
     */
    auto large_buffer = std::unique_ptr<char[]>(
            new char[gkfs::config::rpc::dirents_buff_size_proxy]);

    // We use the full size per server...
    const std::size_t per_host_buff_size =
            gkfs::config::rpc::dirents_buff_size_proxy;
    auto output_ptr = make_unique<
            vector<tuple<const std::string, bool, size_t, time_t>>>();

    // expose local buffers for RMA from servers
    std::vector<hermes::exposed_memory> exposed_buffers;
    exposed_buffers.reserve(1);
    try {
        exposed_buffers.emplace_back(ld_proxy_service->expose(
                std::vector<hermes::mutable_buffer>{hermes::mutable_buffer{
                        large_buffer.get(), per_host_buff_size}},
                hermes::access_mode::write_only));
    } catch(const std::exception& ex) {
        LOG(ERROR, "{}() Failed to expose buffers for RMA. err '{}'", __func__,
            ex.what());
        return make_pair(EBUSY, std::move(output_ptr));
    }

    auto err = 0;
    // send RPCs
    std::vector<hermes::rpc_handle<gkfs::rpc::get_dirents_extended_proxy>>
            handles;

    gkfs::rpc::get_dirents_extended_proxy::input in(path, server,
                                                    exposed_buffers[0]);

    try {
        LOG(DEBUG, "{}() Sending IPC to proxy", __func__);
        handles.emplace_back(
                ld_proxy_service->post<gkfs::rpc::get_dirents_extended_proxy>(
                        endp, in));
    } catch(const std::exception& ex) {
        LOG(ERROR,
            "{}() Unable to send non-blocking proxy_get_dirents() on {} [peer: proxy] err '{}'",
            __func__, path, ex.what());
        err = EBUSY;
    }

    LOG(DEBUG,
        "{}() path '{}' sent rpc_srv_get_dirents() rpc to proxy. Waiting on reply next and deserialize",
        __func__, path);

    // wait for RPC responses

    gkfs::rpc::get_dirents_extended_proxy::output out;

    try {
        // XXX We might need a timeout here to not wait forever for an
        // output that never comes?
        out = handles[0].get().at(0);
        // skip processing dirent data if there was an error during send
        // In this case all responses are gathered but their contents skipped

        if(out.err() != 0) {
            LOG(ERROR,
                "{}() Failed to retrieve dir entries from proxy. Error '{}', path '{}'",
                __func__, strerror(out.err()), path);
            err = out.err();
            // We need to gather all responses before exiting
        }
    } catch(const std::exception& ex) {
        LOG(ERROR,
            "{}() Failed to get rpc output.. [path: {}, target host: proxy] err '{}'",
            __func__, path, ex.what());
        err = EBUSY;
        // We need to gather all responses before exiting
    }

    // The parenthesis is extremely important if not the cast will add as a
    // size_t or a time_t and not as a char
    auto out_buff_ptr = static_cast<char*>(exposed_buffers[0].begin()->data());
    auto bool_ptr = reinterpret_cast<bool*>(out_buff_ptr);
    auto size_ptr = reinterpret_cast<size_t*>(
            (out_buff_ptr) + (out.dirents_size() * sizeof(bool)));
    auto ctime_ptr = reinterpret_cast<time_t*>(
            (out_buff_ptr) +
            (out.dirents_size() * (sizeof(bool) + sizeof(size_t))));
    auto names_ptr =
            out_buff_ptr + (out.dirents_size() *
                            (sizeof(bool) + sizeof(size_t) + sizeof(time_t)));

    for(std::size_t j = 0; j < out.dirents_size(); j++) {

        bool ftype = (*bool_ptr);
        bool_ptr++;

        size_t size = *size_ptr;
        size_ptr++;

        time_t ctime = *ctime_ptr;
        ctime_ptr++;

        auto name = std::string(names_ptr);
        // number of characters in entry + \0 terminator
        names_ptr += name.size() + 1;
        output_ptr->emplace_back(
                std::forward_as_tuple(name, ftype, size, ctime));
    }
    return make_pair(err, std::move(output_ptr));
}

} // namespace rpc
} // namespace gkfs