/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#include <proxy/proxy_data.hpp>

using namespace std;

namespace gkfs {
namespace proxy {

const shared_ptr<spdlog::logger>&
ProxyData::log() const {
    return spdlogger_;
}

void
ProxyData::log(const shared_ptr<spdlog::logger>& log) {
    spdlogger_ = log;
}


margo_instance*
ProxyData::client_rpc_mid() {
    return client_rpc_mid_;
}

void
ProxyData::client_rpc_mid(margo_instance* client_rpc_mid) {
    client_rpc_mid_ = client_rpc_mid;
}

margo_instance*
ProxyData::server_ipc_mid() {
    return server_ipc_mid_;
}

void
ProxyData::server_ipc_mid(margo_instance* server_ipc_mid) {
    server_ipc_mid_ = server_ipc_mid;
}

const string&
ProxyData::server_self_addr() const {
    return server_self_addr_;
}

void
ProxyData::server_self_addr(const string& server_self_addr) {
    server_self_addr_ = server_self_addr;
}

bool
ProxyData::use_auto_sm() const {
    return use_auto_sm_;
}
void
ProxyData::use_auto_sm(bool use_auto_sm) {
    use_auto_sm_ = use_auto_sm;
}

std::map<uint64_t, hg_addr_t>&
ProxyData::rpc_endpoints() {
    return rpc_endpoints_;
}

void
ProxyData::rpc_endpoints(const std::map<uint64_t, hg_addr_t>& rpc_endpoints) {
    rpc_endpoints_ = rpc_endpoints;
}

uint64_t
ProxyData::hosts_size() const {
    return hosts_size_;
}
void
ProxyData::hosts_size(uint64_t hosts_size) {
    hosts_size_ = hosts_size;
}

uint64_t
ProxyData::local_host_id() const {
    return local_host_id_;
}

void
ProxyData::local_host_id(uint64_t local_host_id) {
    local_host_id_ = local_host_id;
}

void
ProxyData::distributor(std::shared_ptr<gkfs::rpc::Distributor> d) {
    distributor_ = d;
}

const string&
ProxyData::pid_file_path() const {
    return pid_file_path_;
}

void
ProxyData::pid_file_path(const string& pid_file_path) {
    pid_file_path_ = pid_file_path;
}

std::shared_ptr<gkfs::rpc::Distributor>
ProxyData::distributor() const {
    return distributor_;
}

margo_client_ids&
ProxyData::rpc_client_ids() {
    return rpc_client_ids_;
}


} // namespace proxy
} // namespace gkfs