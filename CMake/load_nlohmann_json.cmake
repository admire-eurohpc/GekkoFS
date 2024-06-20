################################################################################
# Copyright 2018-2023, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2023, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# This file is part of GekkoFS.                                                #
#                                                                              #
# GekkoFS is free software: you can redistribute it and/or modify              #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation, either version 3 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# GekkoFS is distributed in the hope that it will be useful,                   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.            #
#                                                                              #
# SPDX-License-Identifier: GPL-3.0-or-later                                    #
################################################################################

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(nlohmann_json
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)

FetchContent_GetProperties(nlohmann_json)

if (NOT nlohmann_json_POPULATED)
    FetchContent_Populate(nlohmann_json)
    message(STATUS "[gkfs.io] Nlohmann JSON source dir: ${nlohmann_json_SOURCE_DIR}")
    message(STATUS "[gkfs.io] Nlohmann JSON binary dir: ${nlohmann_json_BINARY_DIR}")

    # we don't really care so much about a third party library's tests to be
    # run from our own project's code
    set(JSON_BuildTests OFF CACHE INTERNAL "")

    # we also don't need to install it when our main project gets installed
    set(JSON_Install OFF CACHE INTERNAL "")

    add_subdirectory(${nlohmann_json_SOURCE_DIR} ${nlohmann_json_BINARY_DIR})
endif ()