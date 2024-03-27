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
#include <nlohmann/json.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

extern "C" {
#include <unistd.h>
#include <fcntl.h>
}

using json = nlohmann::json;

void
report_msgpack(std::vector<unsigned char>& buffer) {
    std::error_code ec{};
    auto undata =
            msgpack::unpack<gkfs::messagepack::ClientMetrics::msgpack_data>(
                    buffer, ec);
    std::vector<double> avg_thruput(undata.req_size_.size());
    for(size_t i = 0; i < avg_thruput.size(); ++i) {
        auto size_mib = undata.req_size_[i] / (1024.0 * 1024.0); // in MiB
        auto duration_s = (undata.end_t_[i] - undata.start_t_[i]) / 1000000.0;
        avg_thruput[i] = std::round((size_mib / duration_s) * 100.0) / 100.0;
    }
    json json_obj;
    json_obj["hostname"] = undata.hostname_;
    json_obj["pid"] = undata.pid_;
    json_obj["io_type"] = undata.io_type_;
    json_obj["total_bytes"] = undata.total_bytes_;
    json_obj["total_iops"] = undata.total_iops_;
    json_obj["start_t_micro"] = undata.start_t_;
    json_obj["end_t_micro"] = undata.end_t_;
    json_obj["flush_t"] = undata.flush_t_;
    json_obj["req_size"] = undata.req_size_;
    json_obj["[extra]avg_thruput_mib"] = avg_thruput;

    std::cout << "Generated JSON:" << '\n';
    for(const auto& item : json_obj.items()) {
        std::cout << item.key() << ": " << item.value().dump() << '\n';
    }
}

void
read_file(const std::string& path) {
    if(!std::filesystem::exists(path)) {
        std::cerr << "Input file " << path << " does not exist" << '\n';
        return;
    }
    std::ifstream file(path, std::ios::binary);
    if(!file.is_open()) {
        std::cout << "failed to open " << '\n';
        return;
    }
    std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());

    file.close();
    report_msgpack(buffer);
}


int
main(int argc, char** argv) {
    std::string mode{};
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <path_to_clientmetrics.msgpack/receiver ip>\n"
                  << " ZMQ listing must start with tcp://<ip:port>\n";
        return -1;
    }
    auto path_ip = std::string(argv[1]);
    if(path_ip.find("tcp://") == 0) {
        // Create a ZeroMQ context
        zmq::context_t context(1);
        // Create a socket to for the server to bind to
        zmq::socket_t socket(context, ZMQ_PULL);
        std::cout << "Binding to: " << path_ip << std::endl;
        socket.bind(path_ip);

        while(true) {
            // Wait for a request message
            zmq::message_t request;
            std::cout << "Waiting for message... " << std::endl;
            if(!socket.recv(request, zmq::recv_flags::none)) {
                std::cerr << "Failed to receive message" << std::endl;
                continue;
            }
            std::cout << "Received message with size " << request.size()
                      << '\n';

            std::vector<unsigned char> requestData(
                    static_cast<unsigned char*>(request.data()),
                    static_cast<unsigned char*>(request.data()) +
                            request.size());

            report_msgpack(requestData);
            std::cout
                    << "-------------------------------------------------------------------------------------\n";
        }
    } else {
        read_file(path_ip);
    }
    //    std::cout << json_obj.dump(4) << std::endl; // Use dump for pretty
    //    printing

    return 0;
}