/*
  Copyright 2018-2023, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2023, Johannes Gutenberg Universitaet Mainz, Germany

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

#ifndef GKFS_COMMON_MSGPACK_HPP
#define GKFS_COMMON_MSGPACK_HPP

#include <msgpack/msgpack.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <csignal>
#include <thread>
#include <mutex>

namespace gkfs::messagepack {

enum class client_metric_type { write, read };

class ClientMetrics {

public:
    //    std::mutex mtx_{};
    //    std::thread thread_{};

    std::chrono::time_point<std::chrono::system_clock> init_t_;
    std::string hostname_;
    int pid_;

    // in milliseconds
    std::vector<double> start_t_{};
    std::vector<double> end_t_{};
    // in bytes per second
    std::vector<double> avg_{};

    uint64_t total_bytes_{};
    int total_iops_{0};

    bool is_enabled_{false};
    std::string path_{};

    // public:
    ClientMetrics();

    ~ClientMetrics() = default;

    template <class T>
    void
    pack(T& pack) {
        pack(init_t_, hostname_, pid_, start_t_, end_t_, avg_, total_iops_,
             total_bytes_);
    }

    void
    add_event(size_t size,
              std::chrono::time_point<std::chrono::system_clock> start);

    void
    flush_msgpack();

    void
    enable();

    void
    disable();

    [[nodiscard]] const std::string&
    path() const;

    void
    path(const std::string& path, const std::string prefix = "");
};

} // namespace gkfs::messagepack

#endif // GKFS_COMMON_MSGPACK_HPP
