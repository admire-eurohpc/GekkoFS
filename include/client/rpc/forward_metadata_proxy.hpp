/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_FORWARD_METADATA_PROXY_HPP
#define GEKKOFS_FORWARD_METADATA_PROXY_HPP

namespace gkfs::rpc {

int
forward_create_proxy(const std::string& path, const mode_t mode);

int
forward_stat_proxy(const std::string& path, std::string& attr);

int
forward_remove_proxy(const std::string& path);

int
forward_decr_size_proxy(const std::string& path, size_t length);

std::pair<int, off64_t>
forward_update_metadentry_size_proxy(const std::string& path, const size_t size,
                                     const off64_t offset,
                                     const bool append_flag);

std::pair<int, off64_t>
forward_get_metadentry_size_proxy(const std::string& path);

std::pair<int, std::vector<std::tuple<const std::string, bool, size_t, time_t>>>
forward_get_dirents_single_proxy(const std::string& path, int server);

} // namespace gkfs::rpc

#endif // GEKKOFS_FORWARD_METADATA_PROXY_HPP
