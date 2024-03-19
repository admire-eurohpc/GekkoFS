/*
  Copyright 2018-2023, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2023, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS' POSIX interface.

  GekkoFS' POSIX interface is free software: you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  GekkoFS' POSIX interface is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with GekkoFS' POSIX interface.  If not, see
  <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: LGPL-3.0-or-later
*/


#include <common/msgpack_util.hpp>
#include <common/rpc/rpc_util.hpp>
#include <cstring>
#include <iostream>
#include <iomanip>

#include <config.hpp>

extern "C" {
#include <fcntl.h>
}

using namespace std;

namespace gkfs::messagepack {

ClientMetrics::ClientMetrics(client_metric_io_type io_type,
                             client_metric_flush_type ftype) {
    msgpack_data_.init_t_ = std::chrono::system_clock::now();
    msgpack_data_.hostname_ = gkfs::rpc::get_my_hostname(true);
    msgpack_data_.pid_ = getpid();
    if(io_type == client_metric_io_type::write) {
        msgpack_data_.io_type_ = "w";
    } else {
        msgpack_data_.io_type_ = "r";
    }
    if(ftype == client_metric_flush_type::file) {
        // set default path
        flush_type_ = client_metric_flush_type::file;
        path_ = gkfs::config::metrics::client_metrics_path;
    } else {
        flush_type_ = client_metric_flush_type::socket;
        zmq_context_ = zmq::context_t(1);
        zmq_socket_ = zmq::socket_t(zmq_context_, ZMQ_PUSH);
    }
}

ClientMetrics::~ClientMetrics() {
    if(flush_type_ == client_metric_flush_type::socket) {
        zmq_socket_.close();
        zmq_context_.close();
    }
}

void
ClientMetrics::add_event(
        size_t size, std::chrono::time_point<std::chrono::system_clock> start) {
    if(!is_enabled_)
        return;
    auto end = std::chrono::system_clock::now();

    auto start_offset = std::chrono::duration<double, std::micro>(
            start - msgpack_data_.init_t_);
    auto end_offset = std::chrono::duration<double, std::micro>(
            end - msgpack_data_.init_t_);
    auto duration = std::chrono::duration<double, std::micro>(end_offset -
                                                              start_offset);

    msgpack_data_.total_bytes_ += size;
    //    auto size_mib = size / (1024 * 1024);      // in MiB
    //    auto duration_s = duration.count() / 1000; // in seconds
    // throw away decimals
    msgpack_data_.start_t_.emplace_back(
            static_cast<size_t>(start_offset.count()));
    msgpack_data_.end_t_.emplace_back(static_cast<size_t>(end_offset.count()));
    msgpack_data_.req_size_.emplace_back(size);
    msgpack_data_.total_iops_ += 1;
}

void
ClientMetrics::flush_msgpack() {
    if(!is_enabled_)
        return;
    auto data = msgpack_data_.pack_msgpack();
    if(flush_type_ == client_metric_flush_type::file) {
        auto fd = open(path_.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
        if(fd < 0) {
            //        cout << "error open" << endl;
            exit(1);
        }
        write(fd, data.data(), data.size());
        //    auto written = write(fd, data.data(), data.size());
        //    cout << "written: " << written << endl;
        close(fd);
    } else {
        zmq::message_t message(data.size());
        // copy data from serialized msgpack to zmq message
        memcpy(message.data(), data.data(), data.size());
        // non-blocking zmq send
        if(zmq_socket_.send(message, zmq::send_flags::none) == -1) {
            std::cerr << "Failed to send zmq message" << std::endl;
        }
    }
}

void
ClientMetrics::enable() {
    is_enabled_ = true;
}

void
ClientMetrics::disable() {
    is_enabled_ = false;
}

void
ClientMetrics::zmq_connect(const string& ip_port) {
    auto address = "tcp://" + ip_port;
    zmq_socket_.connect(address);
}

bool
ClientMetrics::zmq_is_connected() {
    return zmq_socket_.handle() != nullptr;
}

const string&
ClientMetrics::path() const {
    return path_;
}
void
ClientMetrics::path(const string& path, const string prefix) {
    const std::time_t t =
            std::chrono::system_clock::to_time_t(msgpack_data_.init_t_);
    std::stringstream init_t_stream;
    init_t_stream << std::put_time(std::localtime(&t), "%F_%T");
    path_ = path + "/" + prefix + "_" + init_t_stream.str() + "_" +
            msgpack_data_.hostname_ + "_" + to_string(msgpack_data_.pid_) +
            ".msgpack";
}


} // namespace gkfs::messagepack
