/*
  Copyright 2018-2021, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2021, Johannes Gutenberg Universitaet Mainz, Germany

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

#ifndef GEKKOFS_RPC_DISTRIBUTOR_HPP
#define GEKKOFS_RPC_DISTRIBUTOR_HPP

#include <vector>
#include <string>
#include <numeric>

namespace gkfs::rpc {

using chunkid_t = unsigned int;
using host_t = unsigned int;

class Distributor {
public:
    virtual host_t
    localhost() const = 0;

    virtual host_t
    locate_data(const std::string& path, const chunkid_t& chnk_id) const = 0;

    virtual host_t
    locate_file_metadata(const std::string& path) const = 0;

    virtual std::vector<host_t>
    locate_directory_metadata(const std::string& path) const = 0;
};


class SimpleHashDistributor : public Distributor {
private:
    host_t localhost_;
    unsigned int hosts_size_;
    std::vector<host_t> all_hosts_;
    std::hash<std::string> str_hash;

public:
    SimpleHashDistributor(host_t localhost, unsigned int hosts_size);

    host_t
    localhost() const override;

    host_t
    locate_data(const std::string& path,
                const chunkid_t& chnk_id) const override;

    host_t
    locate_file_metadata(const std::string& path) const override;

    std::vector<host_t>
    locate_directory_metadata(const std::string& path) const override;
};

class LocalOnlyDistributor : public Distributor {
private:
    host_t localhost_;

public:
    explicit LocalOnlyDistributor(host_t localhost);

    host_t
    localhost() const override;

    host_t
    locate_data(const std::string& path,
                const chunkid_t& chnk_id) const override;

    host_t
    locate_file_metadata(const std::string& path) const override;

    std::vector<host_t>
    locate_directory_metadata(const std::string& path) const override;
};

class ForwarderDistributor : public Distributor {
private:
    host_t fwd_host_;
    unsigned int hosts_size_;
    std::vector<host_t> all_hosts_;
    std::hash<std::string> str_hash;

public:
    ForwarderDistributor(host_t fwhost, unsigned int hosts_size);

    host_t
    localhost() const override final;

    host_t
    locate_data(const std::string& path,
                const chunkid_t& chnk_id) const override final;

    host_t
    locate_file_metadata(const std::string& path) const override;

    std::vector<host_t>
    locate_directory_metadata(const std::string& path) const override;
};

} // namespace gkfs::rpc

#endif // GEKKOFS_RPC_LOCATOR_HPP