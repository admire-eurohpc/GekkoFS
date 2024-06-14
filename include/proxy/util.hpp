/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_PROXY_UTIL_HPP
#define GEKKOFS_PROXY_UTIL_HPP

#include <string>
#include <vector>

namespace gkfs {
namespace util {

bool
is_proxy_already_running();

void
create_proxy_pid_file();

void
remove_proxy_pid_file();

bool
check_for_hosts_file(const std::string& hostfile);

std::vector<std::pair<std::string, std::string>>
read_hosts_file(const std::string& hostfile);

void
connect_to_hosts(const std::vector<std::pair<std::string, std::string>>& hosts);

} // namespace util
} // namespace gkfs

#endif // GEKKOFS_PROXY_UTIL_HPP
