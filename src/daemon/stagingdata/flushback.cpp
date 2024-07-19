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
 * @brief This file is flushing back data to Lustre at end of the job
 * @internal
 * This file triggers the operation to flush back data to the Lustre.
 * @endinternal
 */
#include "daemon/stagingdata/flushback.hpp"
#include "common/staging/transfer.hpp"

namespace stageout {
    // Define flushback function
    void flushback() {
        // Implementation of flushback function
        // Example: Assume an error occurred
        /*bool errorOccurred = true;
        if (errorOccurred) {
            // Throw an exception
            throw std::runtime_error("Error occurred during flushback operation");
        }*/
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
                    // Call transfer::transferfilegekko with the modified line
                    transfer::transferfilelustre(replaced_line, line);
                }
            }
            file.close();
        } else {
            throw std::runtime_error("Error occurred during flushback files to Lustre");
        }
        }    else {
        throw std::runtime_error("Error no file is availabe");
        }
    }
} // namespace stageout