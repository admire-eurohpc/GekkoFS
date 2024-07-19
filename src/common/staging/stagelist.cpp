
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
/**
 * @brief This file manages prefetch operations 
 * @internal
 * This file includes all operations related to triggering cargo transfer operations
 * @endinternal
 */
#include <client/find_path.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <common/staging/transfer.hpp>


namespace hsminital {

    std::string cached_file_list = "etc/path.config";
    std::ifstream file(cached_file_list);
    std::string lustreroot, gekkoroot; // Variables to store the path of hsm and nvme

    bool transfercachedfiles() {
        std::ifstream file(cached_file_list);
        std::vector<std::string> cached_files;

        if (file.is_open()) {
        std::string line1, line2;
        if (getline(file, line1) && getline(file, line2)) {
            // Read the first two lines
            lustreroot = line1;
            gekkoroot = line2;

            std::string line;
            while (getline(file, line)) {
                // Check if the line starts with lustreroot
                if (line.find(lustreroot) == 0) {
                    // Replace lustreroot with gekkoroot
                    std::string replaced_line = gekkoroot + line.substr(lustreroot.size());
                    // Call transfer::transfer_posix with the modified line
                    bool result = transfer::transferfilegekko(line, replaced_line);
                    if (!result) {
                        std::cerr << "Error: Transfer of file from " << line << " to " << replaced_line << " failed." << std::endl;
                        return false; // Exit with error
                    }
                }
            }
            file.close();
        } else {
            std::cerr << "Error: Unable to read the lines from " << cached_file_list << '\n';
        }
    } else {
        std::cerr << "Error: Unable to open " << cached_file_list << '\n';
    }
        return true;
    }

} // namespace hsminital