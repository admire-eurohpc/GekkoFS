/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_FORWARD_DATA_PROXY_HPP
#define GEKKOFS_FORWARD_DATA_PROXY_HPP

#include <common/common_defs.hpp>

namespace gkfs::rpc {

std::pair<int, ssize_t>
forward_write_proxy(const std::string& path, const void* buf, off64_t offset,
                    size_t write_size);

std::pair<int, ssize_t>
forward_read_proxy(const std::string& path, void* buf, off64_t offset,
                   size_t read_size);

int
forward_truncate_proxy(const std::string& path, size_t current_size,
                       size_t new_size);

std::pair<int, ChunkStat>
forward_get_chunk_stat_proxy();

} // namespace gkfs::rpc

#endif // GEKKOFS_FORWARD_DATA_PROXY_HPP
