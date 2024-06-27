/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_PROXY_FWD_DATA_HPP
#define GEKKOFS_PROXY_FWD_DATA_HPP

#include <proxy/proxy.hpp>

namespace gkfs {
namespace rpc {

std::pair<int, ssize_t>
forward_write(const std::string& path, void* buf, int64_t offset,
              size_t write_size);

std::pair<int, ssize_t>
forward_read(const std::string& path, void* buf, int64_t offset,
             size_t read_size);

int
forward_truncate(const std::string& path, size_t current_size, size_t new_size);

std::pair<int, ChunkStat>
forward_get_chunk_stat();

} // namespace rpc
} // namespace gkfs

#endif // GEKKOFS_PROXY_FWD_DATA_HPP
