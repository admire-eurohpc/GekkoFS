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

#ifndef LFS_RPC_DATA_HPP
#define LFS_RPC_DATA_HPP

#include <daemon/daemon.hpp>

namespace gkfs {

/* Forward declarations */
namespace rpc {
class Distributor;
}


namespace daemon {

struct margo_client_ids {
    hg_id_t test_rpc_id;
};

class RPCData {

private:
    RPCData() {}

    // Margo IDs. They can also be used to retrieve the Mercury classes and
    // contexts that were created at init time
    margo_instance_id server_rpc_mid_;
    margo_instance_id proxy_server_rpc_mid_;
    margo_instance_id client_rpc_mid_;
    margo_client_ids rpc_client_ids_{};

    // Argobots I/O pools and execution streams
    ABT_pool io_pool_;
    std::vector<ABT_xstream> io_streams_;
    std::string self_addr_str_;
    std::string self_proxy_addr_str_;
    // Distributor
    std::shared_ptr<gkfs::rpc::Distributor> distributor_;

public:
    static RPCData*
    getInstance() {
        static RPCData instance;
        return &instance;
    }

    RPCData(RPCData const&) = delete;

    void
    operator=(RPCData const&) = delete;

    // Getter/Setter

    margo_instance*
    server_rpc_mid();

    void
    server_rpc_mid(margo_instance* server_rpc_mid);

    margo_instance*
    proxy_server_rpc_mid();

    void
    proxy_server_rpc_mid(margo_instance* client_rpc_mid);

    margo_instance*
    client_rpc_mid();

    void
    client_rpc_mid(margo_instance* client_rpc_mid);

    margo_client_ids&
    rpc_client_ids();

    ABT_pool
    io_pool() const;

    void
    io_pool(ABT_pool io_pool);

    std::vector<ABT_xstream>&
    io_streams();

    void
    io_streams(const std::vector<ABT_xstream>& io_streams);

    const std::string&
    self_addr_str() const;

    void
    self_addr_str(const std::string& addr_str);

    const std::string&
    self_proxy_addr_str() const;

    void
    self_proxy_addr_str(const std::string& proxy_addr_str);

    const std::shared_ptr<gkfs::rpc::Distributor>&
    distributor() const;

    void
    distributor(const std::shared_ptr<gkfs::rpc::Distributor>& distributor);
};

} // namespace daemon
} // namespace gkfs

#endif // LFS_RPC_DATA_HPP
