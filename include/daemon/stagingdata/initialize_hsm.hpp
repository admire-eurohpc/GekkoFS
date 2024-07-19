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
#ifndef INTIALIZE_HSM_HPP
#define INTIALIZE_HSM_HPP


#include <thread>
#include <iostream>
#include <common/staging/hsm_extension.hpp>
#include <common/staging/stagelist.hpp>
#include "daemon/stagingdata/flushback.hpp"
#include <daemon/stagingdata/intialize_hsm.hpp>

namespace hsmenv {

    /// @brief Transfer cached files
    void transfer_cached_files();

    /// @brief Setup HSM environment
    void setup();

    /// @brief Shutdown HSM environment
    void shutdown();

}


#endif // INTIALIZE_HSM_HPP
