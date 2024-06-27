/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_PROXY_FORWARD_METADATA_HPP
#define GEKKOFS_PROXY_FORWARD_METADATA_HPP

#include <proxy/proxy.hpp>

namespace gkfs::rpc {

int
forward_create(const std::string& path, const mode_t mode);

std::pair<int, std::string>
forward_stat(const std::string& path);

int
forward_remove(const std::string& path);

int
forward_decr_size(const std::string& path, size_t length);

std::pair<int, off64_t>
forward_get_metadentry_size(const std::string& path);

std::pair<int, off64_t>
forward_update_metadentry_size(const std::string& path, const size_t size,
                               const off64_t offset, const bool append_flag);

std::pair<int, size_t>
forward_get_dirents_single(const std::string& path, int server, void* buf,
                           const size_t bulk_size);

} // namespace gkfs::rpc


#endif // GEKKOFS_PROXY_FORWARD_METADATA_HPP
