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
#include <iostream>
#include <iomanip>

extern "C" {
#include <fcntl.h>
}

using namespace std;

namespace gkfs::messagepack {

ClientMetrics::ClientMetrics() {
    init_t_ = std::chrono::system_clock::now();
    hostname_ = gkfs::rpc::get_my_hostname(true);
    pid_ = getpid();
}

void
ClientMetrics::add_event(
        size_t size, std::chrono::time_point<std::chrono::system_clock> start) {
    if(!is_enabled_)
        return;
    auto end = std::chrono::system_clock::now();

    auto start_offset =
            std::chrono::duration<double, std::milli>(start - init_t_);
    auto end_offset = std::chrono::duration<double, std::milli>(end - init_t_);
    auto duration = std::chrono::duration<double, std::milli>(end_offset -
                                                              start_offset);

    total_bytes_ += size;
    //    cout << "duration size bytes: " << size << endl;
    size /= (1024 * 1024); // in MiB
                           //    cout << "duration size MiB: " << size << endl;
    auto duration_s =
            duration.count() /
            1000; // in seconds
                  //    cout << "start: " << start_offset.count() << endl;
                  //    cout << "duration millisecond: " << duration.count() <<
                  //    endl; cout << "duration second: " << duration_s << endl;
                  //    cout << "end: " << end_offset.count() << endl;
    start_t_.emplace_back(start_offset.count());
    end_t_.emplace_back(end_offset.count());
    avg_.emplace_back((size / duration_s));
    total_iops_ += 1;
}

void
ClientMetrics::flush_msgpack() {
    if(!is_enabled_)
        return;
    auto data = msgpack::pack(*this);
    auto fd = open(path_.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
    if(fd < 0) {
        //        cout << "error open" << endl;
        exit(1);
    }
    write(fd, data.data(), data.size());
    //    auto written = write(fd, data.data(), data.size());
    //    cout << "written: " << written << endl;
    close(fd);
}

void
ClientMetrics::enable() {
    is_enabled_ = true;
}

void
ClientMetrics::disable() {
    is_enabled_ = false;
}

const string&
ClientMetrics::path() const {
    return path_;
}
void
ClientMetrics::path(const string& path, const string prefix) {
    const std::time_t t = std::chrono::system_clock::to_time_t(init_t_);
    std::stringstream init_t_stream;
    init_t_stream << std::put_time(std::localtime(&t), "%F_%T");
    path_ = path + "/" + prefix + "_" + init_t_stream.str() + "_" + hostname_ +
            "_" + to_string(pid_) + ".msgpack";
}

} // namespace gkfs::messagepack
