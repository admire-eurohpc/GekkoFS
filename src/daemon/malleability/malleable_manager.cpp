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

#include <daemon/malleability/malleable_manager.hpp>
#include <daemon/malleability/rpc/forward_redistribution.hpp>
#include <daemon/backend/metadata/db.hpp>
#include <daemon/backend/data/chunk_storage.hpp>

#include <common/rpc/rpc_util.hpp>

#include <filesystem>
#include <algorithm>
#include <regex>
#include <random>
#include <thread>

extern "C" {
#include <abt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
}

using namespace std;
namespace fs = std::filesystem;

namespace gkfs::malleable {

vector<pair<string, string>>
MalleableManager::load_hostfile(const std::string& path) {

    GKFS_DATA->spdlogger()->info("{}() Loading hosts file '{}'", __func__,
                                 path);

    ifstream lf(path);
    if(!lf) {
        throw runtime_error(fmt::format("Failed to open hosts file '{}': {}",
                                        path, strerror(errno)));
    }
    vector<pair<string, string>> hosts;
    const regex line_re("^(\\S+)\\s+(\\S+)\\s*(\\S*)$",
                        regex::ECMAScript | regex::optimize);
    string line;
    string host;
    string uri;
    std::smatch match;
    while(getline(lf, line)) {
        // if line starts with #, it indicates the end of current FS instance
        // It is therefore skipped
        if(line[0] == '#')
            continue;
        if(!regex_match(line, match, line_re)) {
            GKFS_DATA->spdlogger()->error(
                    "{}() Unrecognized line format: [path: '{}', line: '{}']",
                    path, line);
            throw runtime_error(
                    fmt::format("unrecognized line format: '{}'", line));
        }
        host = match[1];
        uri = match[2];
        hosts.emplace_back(host, uri);
    }
    if(hosts.empty()) {
        throw runtime_error(
                "Hosts file found but no suitable addresses could be extracted");
    }
    // sort hosts so that data always hashes to the same place
    std::sort(hosts.begin(), hosts.end());
    // remove rootdir suffix from host after sorting as no longer required
    for(auto& h : hosts) {
        auto idx = h.first.rfind("#");
        if(idx != string::npos)
            h.first.erase(idx, h.first.length());
    }
    return hosts;
}

vector<pair<string, string>>
MalleableManager::read_hosts_file() {
    auto hostfile = GKFS_DATA->hosts_file();
    GKFS_DATA->spdlogger()->info("{}() Reading hosts file...", __func__);

    vector<pair<string, string>> hosts;
    try {
        hosts = load_hostfile(hostfile);
    } catch(const exception& e) {
        auto emsg = fmt::format("Failed to load hosts file: {}", e.what());
        throw runtime_error(emsg);
    }

    if(hosts.empty()) {
        throw runtime_error(fmt::format("Hostfile empty: '{}'", hostfile));
    }
    GKFS_DATA->spdlogger()->info("{}() Number of hosts after expansion '{}'",
                                 __func__, hosts.size());
    return hosts;
}

void
MalleableManager::connect_to_hosts(
        const vector<std::pair<std::string, std::string>>& hosts) {
    auto local_hostname = gkfs::rpc::get_my_hostname(true);
    bool local_host_found = false;

    RPC_DATA->hosts_size(hosts.size());
    vector<uint64_t> host_ids(hosts.size());
    // populate vector with [0, ..., host_size - 1]
    ::iota(::begin(host_ids), ::end(host_ids), 0);
    /*
     * Shuffle hosts to balance addr lookups to all hosts
     * Too many concurrent lookups send to same host
     * could overwhelm the server,
     * returning error when addr lookup
     */
    ::random_device rd; // obtain a random number from hardware
    ::mt19937 g(rd());  // seed the random generator
    ::shuffle(host_ids.begin(), host_ids.end(), g); // Shuffle hosts vector
    // lookup addresses and put abstract server addresses into rpc_addresses
    for(const auto& id : host_ids) {
        const auto& hostname = hosts.at(id).first;
        const auto& uri = hosts.at(id).second;

        hg_addr_t svr_addr = HG_ADDR_NULL;

        // try to look up 3 times before erroring out
        hg_return_t ret;
        for(uint32_t i = 0; i < 4; i++) {
            ret = margo_addr_lookup(RPC_DATA->client_rpc_mid(), uri.c_str(),
                                    &svr_addr);
            if(ret != HG_SUCCESS) {
                // still not working after 5 tries.
                if(i == 3) {
                    auto err_msg =
                            fmt::format("{}() Unable to lookup address '{}'",
                                        __func__, uri);
                    throw runtime_error(err_msg);
                }
                // Wait a random amount of time and try again
                ::mt19937 eng(rd()); // seed the random generator
                ::uniform_int_distribution<> distr(
                        50, 50 * (i + 2)); // define the range
                ::this_thread::sleep_for(std::chrono::milliseconds(distr(eng)));
            } else {
                break;
            }
        }
        if(svr_addr == HG_ADDR_NULL) {
            auto err_msg = fmt::format(
                    "{}() looked up address is NULL for address '{}'", __func__,
                    uri);
            throw runtime_error(err_msg);
        }
        RPC_DATA->rpc_endpoints().insert(make_pair(id, svr_addr));

        if(!local_host_found && hostname == local_hostname) {
            GKFS_DATA->spdlogger()->debug("{}() Found local host: {}", __func__,
                                          hostname);
            RPC_DATA->local_host_id(id);
            local_host_found = true;
        }
        GKFS_DATA->spdlogger()->debug("{}() Found daemon: id '{}' uri '{}'",
                                      __func__, id, uri);
    }
    if(!local_host_found) {
        auto err_msg = fmt::format(
                "{}() Local host '{}' not found in hosts file. This should not happen.",
                __func__, local_hostname);
        throw runtime_error(err_msg);
    }
}

void
MalleableManager::expand_abt(void* _arg) {
    GKFS_DATA->spdlogger()->info("{}() Starting expansion process...",
                                 __func__);
    GKFS_DATA->redist_running(true);
    GKFS_DATA->malleable_manager()->redistribute_metadata();
    try {
        GKFS_DATA->malleable_manager()->redistribute_data();
    } catch(const gkfs::data::ChunkStorageException& e) {
        GKFS_DATA->spdlogger()->error("{}() Failed to redistribute data: '{}'",
                                      __func__, e.what());
    }
    GKFS_DATA->redist_running(false);
    GKFS_DATA->spdlogger()->info(
            "{}() Expansion process successfully finished.", __func__);
}

void
MalleableManager::expand_start(int old_server_conf, int new_server_conf) {
    auto hosts = read_hosts_file();
    if(hosts.size() != static_cast<size_t>(new_server_conf)) {
        throw runtime_error(
                fmt::format("MalleableManager::{}() Something is wrong. "
                            "Number of hosts in hosts file ({}) "
                            "does not match new server configuration ({})",
                            __func__, hosts.size(), new_server_conf));
    }
    connect_to_hosts(hosts);
    RPC_DATA->distributor()->hosts_size(hosts.size());
    auto abt_err =
            ABT_thread_create(RPC_DATA->io_pool(), expand_abt,
                              ABT_THREAD_ATTR_NULL, nullptr, &redist_thread_);
    if(abt_err != ABT_SUCCESS) {
        auto err_str = fmt::format(
                "MalleableManager::{}() Failed to create ABT thread with abt_err '{}'",
                __func__, abt_err);
        throw runtime_error(err_str);
    }
}

int
MalleableManager::redistribute_metadata() {
    uint64_t count = 0;
    auto estimate_db_size = GKFS_DATA->mdb()->db_size();
    auto percent_interval = estimate_db_size / 1000;
    GKFS_DATA->spdlogger()->info(
            "{}() Starting metadata redistribution for '{}' estimated number of KV pairs...",
            __func__, estimate_db_size);
    int migration_err = 0;
    string key, value;
    auto iter =
            static_cast<rocksdb::Iterator*>(GKFS_DATA->mdb()->iterate_all());
    for(iter->SeekToFirst(); iter->Valid(); iter->Next()) {
        key = iter->key().ToString();
        value = iter->value().ToString();
        if(key == "/") {
            continue;
        }
        auto dest_id = RPC_DATA->distributor()->locate_file_metadata(key, 0);
        GKFS_DATA->spdlogger()->info(
                "{}() Migration: key {} and value {}. From host {} to host {}",
                __func__, key, value, RPC_DATA->local_host_id(), dest_id);
        if(dest_id == RPC_DATA->local_host_id()) {
            GKFS_DATA->spdlogger()->info("{}() SKIPPERS", __func__);
            continue;
        }
        auto err = gkfs::malleable::rpc::forward_metadata(key, value, dest_id);
        if(err != 0) {
            GKFS_DATA->spdlogger()->error(
                    "{}() Failed to migrate metadata for key '{}'", __func__,
                    key);
            migration_err++;
        }
        GKFS_DATA->mdb()->remove(key);
        count++;
        if(percent_interval > 0 && count % percent_interval == 0) {
            GKFS_DATA->spdlogger()->info(
                    "{}() Metadata migration {}%/100% completed...", __func__,
                    count / percent_interval);
        }
    }
    GKFS_DATA->spdlogger()->info("{}() Metadata redistribution completed.",
                                 __func__);
    return migration_err;
}

void
MalleableManager::redistribute_data() {
    GKFS_DATA->spdlogger()->info("{}() Starting data redistribution...",
                                 __func__);

    auto chunk_dir = fs::path(GKFS_DATA->storage()->get_chunk_directory());
    auto dir_iterator = GKFS_DATA->storage()->get_all_chunk_files();

    for(const auto& entry : dir_iterator) {
        if(!entry.is_regular_file()) {
            continue;
        }
        // path under chunkdir as placed in the rootdir
        auto rel_chunk_dir = fs::relative(entry, chunk_dir);
        // chunk id from this entry used for determining destination
        uint64_t chunk_id = stoul(rel_chunk_dir.filename().string());
        // mountdir gekkofs path used for determining destination
        auto gkfs_path = rel_chunk_dir.parent_path().string();
        ::replace(gkfs_path.begin(), gkfs_path.end(), ':', '/');
        gkfs_path = "/" + gkfs_path;
        auto dest_id =
                RPC_DATA->distributor()->locate_data(gkfs_path, chunk_id, 0);
        GKFS_DATA->spdlogger()->trace(
                "{}() Migrating chunkfile: {} for gkfs file {} chnkid {} destid {}",
                __func__, rel_chunk_dir.string(), gkfs_path, chunk_id, dest_id);
        if(dest_id == RPC_DATA->local_host_id()) {
            GKFS_DATA->spdlogger()->trace("{}() SKIPPERS", __func__);
            continue;
        }
        auto fd = open(entry.path().c_str(), O_RDONLY);
        if(fd < 0) {
            GKFS_DATA->spdlogger()->error("{}() Failed to open chunkfile: {}",
                                          __func__, entry.path().c_str());
            continue;
        }
        auto buf = new char[entry.file_size()];
        auto bytes_read = read(fd, buf, entry.file_size());
        if(bytes_read < 0) {
            GKFS_DATA->spdlogger()->error("{}() Failed to read chunkfile: {}",
                                          __func__, entry.path().c_str());
            continue;
        }
        auto err = gkfs::malleable::rpc::forward_data(
                gkfs_path, buf, bytes_read, chunk_id, dest_id);
        if(err != 0) {
            GKFS_DATA->spdlogger()->error(
                    "{}() Failed to migrate data for chunkfile: {}", __func__,
                    entry.path().c_str());
        }
        close(fd);
        GKFS_DATA->spdlogger()->trace(
                "{}() Data migration completed for chunkfile: {}. Removing ...",
                __func__, entry.path().c_str());
        // remove file after migration
        auto entry_dir = entry.path().parent_path();
        try {
            fs::remove(entry);
            if(fs::is_empty(entry_dir)) {
                fs::remove(entry_dir);
            }
        } catch(const fs::filesystem_error& e) {
            GKFS_DATA->spdlogger()->error("{}() Failed to remove chunkfile: {}",
                                          __func__, entry.path().c_str());
        }
        GKFS_DATA->spdlogger()->trace("{}() Done for chunkfile: {}", __func__,
                                      entry.path().c_str());
    }

    GKFS_DATA->spdlogger()->info("{}() Data redistribution completed.",
                                 __func__);
}

} // namespace gkfs::malleable
