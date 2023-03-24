/*
  Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany

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

/* C++ includes */
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <fmt/format.h>
#include <commands.hpp>
#include <reflection.hpp>
#include <serialize.hpp>
#include <binary_buffer.hpp>

/* C includes */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>

using json = nlohmann::json;

struct pwritev_options {
    bool verbose{};
    std::string pathname;
    std::string data_0, data_1;
    ::size_t count;
    ::size_t offset;

    REFL_DECL_STRUCT(pwritev_options, REFL_DECL_MEMBER(bool, verbose),
                     REFL_DECL_MEMBER(std::string, pathname),
                     REFL_DECL_MEMBER(std::string, data_0),
                     REFL_DECL_MEMBER(std::string, data_1),
                     REFL_DECL_MEMBER(::size_t, count),
                     REFL_DECL_MEMBER(::size_t, offset));
};

struct pwritev_output {
    ::ssize_t retval;
    int errnum;

    REFL_DECL_STRUCT(pwritev_output, REFL_DECL_MEMBER(::size_t, retval),
                     REFL_DECL_MEMBER(int, errnum));
};

void
to_json(json& record, const pwritev_output& out) {
    record = serialize(out);
}

void
pwritev_exec(const pwritev_options& opts) {

    auto fd = ::open(opts.pathname.c_str(), O_WRONLY);

    if(fd == -1) {
        if(opts.verbose) {
            fmt::print(
                    "pwritev(pathname=\"{}\", buf_0=\"{}\" buf_1=\"{}\" count={}, offset={}) = {}, errno: {} [{}]\n",
                    opts.pathname, opts.data_0, opts.data_1, opts.count,
                    opts.offset, fd, errno, ::strerror(errno));
            return;
        }

        json out = pwritev_output{fd, errno};
        fmt::print("{}\n", out.dump(2));

        return;
    }

    io::buffer buf_0(opts.data_0);
    io::buffer buf_1(opts.data_1);

    struct iovec iov[2];

    iov[0].iov_base = buf_0.data();
    iov[1].iov_base = buf_1.data();

    iov[0].iov_len = buf_0.size();
    iov[1].iov_len = buf_1.size();

    int rv = ::pwritev(fd, iov, opts.count, opts.offset);

    if(opts.verbose) {
        fmt::print(
                "pwritev(pathname=\"{}\", count={}, offset={}) = {}, errno: {} [{}]\n",
                opts.pathname, opts.count, opts.offset, rv, errno,
                ::strerror(errno));
        return;
    }

    json out = pwritev_output{rv, errno};
    fmt::print("{}\n", out.dump(2));
}

void
pwritev_init(CLI::App& app) {

    // Create the option and subcommand objects
    auto opts = std::make_shared<pwritev_options>();
    auto* cmd =
            app.add_subcommand("pwritev", "Execute the pwritev() system call");

    // Add options to cmd, binding them to opts
    cmd->add_flag("-v,--verbose", opts->verbose,
                  "Produce human writeable output");

    cmd->add_option("pathname", opts->pathname, "Directory name")
            ->required()
            ->type_name("");

    cmd->add_option("data_0", opts->data_0, "Data 0 to write")
            ->required()
            ->type_name("");

    cmd->add_option("data_1", opts->data_1, "Data 1 to write")
            ->required()
            ->type_name("");

    cmd->add_option("count", opts->count, "Number of bytes to write")
            ->required()
            ->type_name("");

    cmd->add_option("offset", opts->offset, "Offset to read")
            ->required()
            ->type_name("");

    cmd->callback([opts]() { pwritev_exec(*opts); });
}
