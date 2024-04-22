/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_PROXY_PROXY_DATA_HPP
#define GEKKOFS_PROXY_PROXY_DATA_HPP

#include <proxy/proxy.hpp>
#include <map>


namespace gkfs {
namespace rpc {
class Distributor;
}
namespace proxy {

struct margo_client_ids {
    hg_id_t rpc_create_id;
    hg_id_t rpc_stat_id;
    hg_id_t rpc_remove_id;
    hg_id_t rpc_decr_size_id;
    hg_id_t rpc_remove_data_id;
    hg_id_t rpc_update_metadentry_size_id;
    hg_id_t rpc_write_id;
    hg_id_t rpc_read_id;
    hg_id_t rpc_truncate_id;
    hg_id_t rpc_chunk_stat_id;
    hg_id_t rpc_get_dirents_extended_id;
};

class ProxyData {

private:
    ProxyData() {}

    // logger
    std::shared_ptr<spdlog::logger> spdlogger_{};

    // RPC stuff
    margo_instance_id client_rpc_mid_{};
    margo_instance_id server_ipc_mid_{};
    std::string server_self_addr_{};

    bool use_auto_sm_{false};

    std::map<uint64_t, hg_addr_t> rpc_endpoints_;
    uint64_t hosts_size_;
    uint64_t local_host_id_;

    margo_client_ids rpc_client_ids_{};

    // pid file
    std::string pid_file_path_{gkfs::config::proxy::pid_path};

    // data distribution
    std::shared_ptr<gkfs::rpc::Distributor> distributor_;

public:
    static ProxyData*
    getInstance() {
        static ProxyData instance;
        return &instance;
    }

    ProxyData(ProxyData const&) = delete;

    void
    operator=(ProxyData const&) = delete;

    // Getter/Setter

    const std::shared_ptr<spdlog::logger>&
    log() const;

    void
    log(const std::shared_ptr<spdlog::logger>& log);

    margo_instance*
    client_rpc_mid();

    void
    client_rpc_mid(margo_instance* client_rpc_mid);

    margo_instance*
    server_ipc_mid();

    void
    server_ipc_mid(margo_instance* server_ipc_mid);

    const std::string&
    server_self_addr() const;

    void
    server_self_addr(const std::string& server_self_addr);

    bool
    use_auto_sm() const;
    void
    use_auto_sm(bool use_auto_sm);

    std::map<uint64_t, hg_addr_t>&
    rpc_endpoints();

    void
    rpc_endpoints(const std::map<uint64_t, hg_addr_t>& rpc_endpoints);

    uint64_t
    hosts_size() const;

    void
    hosts_size(uint64_t hosts_size);

    uint64_t
    local_host_id() const;

    void
    local_host_id(uint64_t local_host_id);

    margo_client_ids&
    rpc_client_ids();

    const std::string&
    pid_file_path() const;

    void
    pid_file_path(const std::string& pid_file_path);

    void
    distributor(std::shared_ptr<gkfs::rpc::Distributor> distributor);

    std::shared_ptr<gkfs::rpc::Distributor>
    distributor() const;
};

} // namespace proxy
} // namespace gkfs

#endif // GEKKOFS_PROXY_PROXY_DATA_HPP