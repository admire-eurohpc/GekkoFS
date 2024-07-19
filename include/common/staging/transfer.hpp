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

#ifndef TRANSFER_HPP
#define TRANSFER_HPP

#include <vector>
#include <string>
#include <cargo.hpp>


namespace transfer {

extern int count_lock;
extern int root_lock;
extern std::string srcroot;
extern std::string dstroot;

/// @brief Initialize the Cargo server
/// @return Address of the initialized server
std::string get_initialized_server();

/// @brief Transfer a file from Lustre to GekkoFS
/// @param lustrepath Address of the file in Lustre
/// @param gekkopath Address of the file in GekkoFS
/// @return Success or failure
bool transferfilegekko(const std::string& lustrepath, const std::string& gekkopath);

/// @brief Transfer files from Lustre to GekkoFS
/// @param lustrepaths Addresses of the files in Lustre
/// @param gekkopaths Addresses of the files in GekkoFS
/// @return Success or failure
bool transferdirgekko(const std::vector<std::string>& lustrepaths, const std::vector<std::string>& gekkopaths);

/// @brief Transfer files from GekkoFS to Lustre
/// @param gekkopaths Addresses of the files in GekkoFS
/// @param lustrepaths Addresses of the files in Lustre
/// @return Success or failure
bool transferdirlustre(const std::vector<std::string>& gekkopaths, const std::vector<std::string>& lustrepaths);

/// @brief Transfer a file from GekkoFS to Lustre
/// @param gekkopath Address of the file in GekkoFS
/// @param lustrepath Address of the file in Lustre
/// @return Success or failure
bool transferfilelustre(const std::string& gekkopath, const std::string& lustrepath);

/// @brief Initialize root paths
void intializeroot();

/// @brief Transfer a file based on Lustre root
/// @param path Path of the file to transfer
/// @return Success or failure
bool transferlustre(const std::string& path);

/// @brief Transfer a file based on GekkoFS root
/// @param path Path of the file to transfer
/// @return Success or failure
bool transfergekko(const std::string& path);

} // namespace transfer

#endif // TRANSFER_H
