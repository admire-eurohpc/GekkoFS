/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <proxy/util.hpp>
#include <proxy/proxy.hpp>
#include <proxy/env.hpp>

#include <common/env_util.hpp>
#include <common/rpc/rpc_util.hpp>

#include <filesystem>
#include <csignal>
#include <regex>
#include <random>
#include <fstream>
#include <thread>

extern "C" {
#include <unistd.h>
}

using namespace std;
namespace fs = std::filesystem;

namespace {

vector<pair<string, string>>
load_hostfile(const std::string& lfpath) {


    PROXY_DATA->log()->debug("{}() Loading hosts file: '{}'", __func__, lfpath);

    ifstream lf(lfpath);
    if(!lf) {
        throw runtime_error(fmt::format("Failed to open hosts file '{}': {}",
                                        lfpath, strerror(errno)));
    }
    vector<pair<string, string>> hosts;
    const regex line_re("^(\\S+)\\s+(\\S+)\\s*(\\S*)$",
                        regex::ECMAScript | regex::optimize);
    string line;
    string host;
    string uri;
    std::smatch match;
    while(getline(lf, line)) {
        if(line[0] == '#')
            continue;
        if(!regex_match(line, match, line_re)) {
            PROXY_DATA->log()->debug(
                    "{}() Unrecognized line format: [path: '{}', line: '{}']",
                    __func__, lfpath, line);

            throw runtime_error(
                    fmt::format("unrecognized line format: '{}'", line));
        }
        host = match[1];
        if(match.size() < 3) {
            throw runtime_error(fmt::format(
                    "hostfile does not have three columns for daemon RPC proxy server"));
        }
        uri = match[3];
        if(!PROXY_DATA->use_auto_sm() &&
           uri.find("na+sm") != std::string::npos) {
            PROXY_DATA->use_auto_sm(true);
            PROXY_DATA->log()->info(
                    "{}() auto_sm detected in daemon hosefile. Enabling it on proxy ...",
                    __func__);
        }

        hosts.emplace_back(host, uri);
    }
    return hosts;
}
} // namespace

namespace gkfs::util {

bool
is_proxy_already_running() {
    const auto& pid_path = PROXY_DATA->pid_file_path();

    // check if another proxy is already running
    if(fs::exists(pid_path)) {
        ifstream ifs(pid_path, ::ifstream::in);
        if(ifs) {
            string running_pid{};
            if(getline(ifs, running_pid) && !running_pid.empty()) {
                // check if process exists without killing it. Signal 0 doesn't
                // kill
                if(0 == ::kill(::stoi(running_pid), 0))
                    return true;
            }
        } else {
            throw runtime_error(
                    "FATAL: pid file of another proxy already exists, but cannot be opened. Exiting ...");
        }
        ifs.close();
        fs::remove(pid_path);
    }
    return false;
}

/**
 * Create pid file with na+sm address.
 * At the moment this is not NUMA-aware.
 * E.g., if two PSM2 devices exist, one on each socket, it would be best to use
 * two proxies but since PSM2_MULTIRAIL 2 doesn't work properly, this is a
 * future TODO
 */
void
create_proxy_pid_file() {
    /*
     * - na+sm pid address
     * - file name socket (numa node) getcpu() call in #include <linux/getcpu.h>
     * - only allow one per socket
     */
    const auto& pid_path = PROXY_DATA->pid_file_path();
    auto my_pid = getpid();
    if(my_pid == -1) {
        throw runtime_error("Unable to get own pid for proxy pid file");
    }
    ofstream ofs(pid_path, ::ofstream::trunc);
    if(ofs) {
        ofs << to_string(my_pid);
        ofs << "\n";
        ofs << PROXY_DATA->server_self_addr();
    } else {
        throw runtime_error("Unable to create proxy pid file");
    }
}

void
remove_proxy_pid_file() {
    const auto& pid_path = PROXY_DATA->pid_file_path();
    fs::remove(pid_path);
}

bool
check_for_hosts_file(const std::string& hostfile) {
    return fs::exists(hostfile);
}


vector<pair<string, string>>
read_hosts_file(const std::string& hostfile) {

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

    PROXY_DATA->log()->info("{}() Daemon pool size: '{}'", __func__,
                            hosts.size());

    return hosts;
}

void
connect_to_hosts(const vector<pair<string, string>>& hosts) {
    auto local_hostname = gkfs::rpc::get_my_hostname(true);
    bool local_host_found = false;

    PROXY_DATA->hosts_size(hosts.size());
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
            ret = margo_addr_lookup(PROXY_DATA->client_rpc_mid(), uri.c_str(),
                                    &svr_addr);
            if(ret != HG_SUCCESS) {
                // still not working after 5 tries.
                if(i == 4) {
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
        PROXY_DATA->rpc_endpoints().insert(make_pair(id, svr_addr));

        if(!local_host_found && hostname == local_hostname) {
            PROXY_DATA->log()->debug("{}() Found local host: {}", __func__,
                                     hostname);
            PROXY_DATA->local_host_id(id);
            local_host_found = true;
        }
        PROXY_DATA->log()->debug("{}() Found daemon: id '{}' uri '{}'",
                                 __func__, id, uri);
    }

    if(!local_host_found) {
        PROXY_DATA->log()->warn(
                "{}() Failed to find local host. Using host '0' as local host",
                __func__);
        PROXY_DATA->local_host_id(0);
    }
}

} // namespace gkfs::util