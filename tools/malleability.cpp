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

#include <iostream>
// #include <queue>
#include <string>

#include <CLI/CLI.hpp>
#include <client/user_functions.hpp>


using namespace std;

struct cli_options {
    string hosts_file;
};

int
main(int argc, const char* argv[]) {
    CLI::App desc{"Allowed options"};
    cli_options opts{};

    auto err = gkfs_init();
    cout << "Init result " << err << endl;

    err = gkfs::malleable::expand_start(1, 1);
    if(err) {
        cout << "Expand start failed. Exiting..." << endl;
        gkfs_end();
        return -1;
    }
    cout << "Expand start " << err << endl;
    err = gkfs::malleable::expand_status();
    cout << "Expand status " << err << endl;
    err = gkfs::malleable::expand_finalize();
    cout << "Expand finalize " << err << endl;

    err = gkfs_end();
    cout << "End result " << err << endl;
}