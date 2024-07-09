/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

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

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include <CLI/CLI.hpp>
#include <client/user_functions.hpp>


using namespace std;

struct cli_options {
    bool verbose = false;
    bool machine_readable = false;
    string action;
    string subcommand;
};

std::pair<int, int>
get_expansion_host_num() {
    // get hosts file and read how much should be expanded
    auto hosts_file_path = std::getenv("LIBGKFS_HOSTS_FILE");
    if(!hosts_file_path) {
        std::cerr
                << "Error: LIBGKFS_HOSTS_FILE environment variable not set.\n";
        return {-1, -1};
    }
    std::ifstream file(hosts_file_path);
    if(!file) {
        std::cerr << "Error: Unable to open file at " << hosts_file_path
                  << ".\n";
        return {-1, -1}; // Indicate an error
    }
    auto initialHostCount = 0;
    auto finalHostCount = 0;
    auto foundSeparator = false;
    std::string line;

    while(std::getline(file, line)) {
        if(line == "#FS_INSTANCE_END") {
            if(foundSeparator) {
                cerr << "marker was found twice. this is not allowed.\n";
                return {-1, -1};
            }
            foundSeparator = true;
            initialHostCount = finalHostCount;
            continue;
        }
        if(!line.empty()) {
            finalHostCount++;
        }
    }
    if(!foundSeparator) {
        initialHostCount = finalHostCount;
    }
    return {initialHostCount, finalHostCount};
}

int
main(int argc, const char* argv[]) {
    CLI::App desc{"Allowed options"};
    cli_options opts;

    // Global verbose flag
    desc.add_flag("--verbose,-v", opts.verbose, "Verbose output");
    desc.add_flag("--machine-readable,-m", opts.machine_readable,
                  "machine-readable output");


    auto expand_args =
            desc.add_subcommand("expand", "Expansion-related actions");
    expand_args->add_option("action", opts.action, "Action to perform")
            ->required()
            ->check(CLI::IsMember({"start", "status", "finalize"}));
    try {
        desc.parse(argc, argv);
    } catch(const CLI::ParseError& e) {
        return desc.exit(e);
    }

    if(opts.verbose) { // Check the verbose flag from the main options
        std::cout << "Verbose mode is on." << std::endl;
    }
    int res;
    gkfs_init();

    if(opts.action == "start") {
        auto [current_instance, expanded_instance] = get_expansion_host_num();
        if(current_instance == -1 || expanded_instance == -1) {
            return 1;
        }
        res = gkfs::malleable::expand_start(current_instance,
                                            expanded_instance);
        if(res) {
            cout << "Expand start failed. Exiting...\n";
            gkfs_end();
            return -1;
        } else {
            cout << "Expansion process from " << current_instance
                 << " nodes to " << expanded_instance << " nodes launched...\n";
        }
    } else if(opts.action == "status") {
        res = gkfs::malleable::expand_status();
        if(res > 0) {
            if(opts.machine_readable) {
                cout << res;
            } else {
                cout << "Expansion in progress: " << res
                     << " nodes not finished.\n";
            }
        } else {
            if(opts.machine_readable) {
                cout << res;
            } else {
                cout << "No expansion running/finished.\n";
            }
        }
    } else if(opts.action == "finalize") {
        res = gkfs::malleable::expand_finalize();
        if(opts.machine_readable) {
            cout << res;
        } else {
            cout << "Expand finalize " << res << endl;
        }
    }
    gkfs_end();
}