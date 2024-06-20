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
#include <atomic>
#include <memory>

#include <zmq.hpp>
#include <condition_variable>

namespace gkfs::messagepack {

enum class client_metric_io_type { write, read };
enum class client_metric_flush_type { file, socket };

class ClientMetrics {

public:
    /*
     * MessagePack data structure for client metrics. Includes only what is
     * actually sent
     */
    struct msgpack_data {
        uint32_t flush_t_;
        std::string hostname_;
        int pid_;
        std::string io_type_;
        std::vector<uint32_t> start_t_{};
        std::vector<uint32_t> end_t_{};
        std::vector<uint32_t> req_size_{};
        uint32_t total_bytes_{};
        int total_iops_{0};

        template <class T>
        void
        pack(T& pack) {
            pack(flush_t_, hostname_, pid_, io_type_, start_t_, end_t_,
                 req_size_, total_iops_, total_bytes_);
        }

        std::vector<uint8_t>
        pack_msgpack() {
            return msgpack::pack(*this);
        }
    };

private:
    bool metrics_enabled_{false};

    msgpack_data msgpack_data_{};

    // Initialization time used to compute relative timestamps
    std::chrono::time_point<std::chrono::system_clock> init_t_;

    std::mutex data_mtx_{};
    std::thread flush_thread_{};
    std::condition_variable flush_thread_cv_{};
    std::mutex flush_thread_cv_mutex_{};
    std::atomic<bool> flush_thread_running_{false};

    client_metric_flush_type flush_type_{client_metric_flush_type::file};
    int flush_interval_{};
    std::unique_ptr<zmq::context_t> zmq_flush_context_ = nullptr;
    std::unique_ptr<zmq::socket_t> zmq_flush_socket_ = nullptr;
    std::string flush_path_{};
    int flush_count_{0};


public:
    ClientMetrics() = default;

    explicit ClientMetrics(client_metric_io_type io_type,
                           client_metric_flush_type flush_type =
                                   client_metric_flush_type::file,
                           int flush_interval = 5);

    ~ClientMetrics();

    void
    add_event(size_t size,
              std::chrono::time_point<std::chrono::system_clock> start);

    void
    reset_metrics();

    void
    flush_msgpack();

    void
    flush_loop();

    void
    enable();

    void
    disable();

    void
    zmq_connect(const std::string& ip_port);

    bool
    zmq_is_connected();

    [[nodiscard]] const std::string&
    path() const;

    void
    path(const std::string& path, const std::string prefix = "");

    int
    flush_count() const;
};

} // namespace gkfs::messagepack

#endif // GKFS_COMMON_MSGPACK_HPP
