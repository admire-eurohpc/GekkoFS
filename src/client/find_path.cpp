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
#include <client/find_path.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <client/transfer.hpp>

std::vector<std::string> cacehd_files;

void init_cached_files() {
  std::string cached_file_list = "etc/path.config";
  std::ifstream file(cached_file_list);
  std::string line;
  if (file.is_open()) {
    while (getline(file, line)) {
      cacehd_files.push_back(line);
    }
    file.close();
    //move cache list file from Lustre to Gekkofs
    transfer::transfer_posix(cacehd_files);
  } else {
    std::cout << "Unable to open " << forbidden_file_list << '\n';
  }
}

bool cacehd_file(const std::string &path) {
  static bool initialized = false;
  if (!initialized) {
    init_cached_files();
    initialized = true;
  }
  for (const auto &cacehd_file : cacehd_files) {
    if (path == cacehd_file) {
      return true;
    }
  }
  return false;
}

