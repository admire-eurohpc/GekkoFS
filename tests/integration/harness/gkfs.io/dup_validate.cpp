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
#include <fcntl.h>
#include <unistd.h>

using json = nlohmann::json;

struct dup_validate_options {
    bool verbose{};
    std::string pathname;


    REFL_DECL_STRUCT(dup_validate_options, REFL_DECL_MEMBER(bool, verbose),
                     REFL_DECL_MEMBER(std::string, pathname));
};

struct dup_validate_output {
    int retval;
    int errnum;

    REFL_DECL_STRUCT(dup_validate_output, REFL_DECL_MEMBER(int, retval),
                     REFL_DECL_MEMBER(int, errnum));
};

void
to_json(json& record, const dup_validate_output& out) {
    record = serialize(out);
}

void
dup_validate_exec(const dup_validate_options& opts) {

    int fd = ::open(opts.pathname.c_str(), O_WRONLY);

    if(fd == -1) {
        if(opts.verbose) {
            fmt::print(
                    "dup_validate(pathname=\"{}\") = {}, errno: {} [{}]\n",
                    opts.pathname, fd, errno, ::strerror(errno));
            return;
        }

        json out = dup_validate_output{fd, errno};
        fmt::print("{}\n", out.dump(2));

        return;
    }


    auto rv = ::dup(fd);
    auto rv2 = ::dup2(fd, rv);
    


    if(opts.verbose) {
        fmt::print(
                "dup_validate(pathname=\"{}\") = {}, errno: {} [{}]\n",
                opts.pathname, rv, errno, ::strerror(errno));
        return;
    }

    if(rv < 0 || rv2 < 0) {
        json out = dup_validate_output{(int) rv, errno};
        fmt::print("{}\n", out.dump(2));
        return;
    }

    rv = 0;
    errno = 0;
    json out = dup_validate_output{(int) rv, errno};
    fmt::print("{}\n", out.dump(2));
    return;
}

void
dup_validate_init(CLI::App& app) {

    // Create the option and subcommand objects
    auto opts = std::make_shared<dup_validate_options>();
    auto* cmd = app.add_subcommand(
            "dup_validate",
            "Execute dup, dup2, and dup3 returns 0 if the file descriptor is valid");

    // Add options to cmd, binding them to opts
    cmd->add_flag("-v,--verbose", opts->verbose,
                  "Produce human writeable output");

    cmd->add_option("pathname", opts->pathname, "File name")
            ->required()
            ->type_name("");

    cmd->callback([opts]() { dup_validate_exec(*opts); });
}
