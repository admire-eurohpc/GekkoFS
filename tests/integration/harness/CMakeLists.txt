################################################################################
# Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany          #
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

cmake_minimum_required(VERSION 3.11)

project(gkfs.io
    VERSION 0.1
    LANGUAGES CXX
)

add_executable(gkfs.io
    gkfs.io/main.cpp
    gkfs.io/commands.hpp
    gkfs.io/mkdir.cpp
    gkfs.io/open.cpp
    gkfs.io/opendir.cpp
    gkfs.io/read.cpp
    gkfs.io/readv.cpp
    gkfs.io/pread.cpp
    gkfs.io/preadv.cpp
    gkfs.io/readdir.cpp
    gkfs.io/reflection.hpp
    gkfs.io/rmdir.cpp
    gkfs.io/serialize.hpp
    gkfs.io/stat.cpp
    gkfs.io/write.cpp
    gkfs.io/pwrite.cpp
    gkfs.io/writev.cpp
    gkfs.io/pwritev.cpp
    gkfs.io/statx.cpp
    gkfs.io/lseek.cpp
    gkfs.io/write_validate.cpp
    gkfs.io/write_random.cpp
    gkfs.io/truncate.cpp
    gkfs.io/util/file_compare.cpp
    gkfs.io/chdir.cpp
    gkfs.io/getcwd_validate.cpp
    gkfs.io/symlink.cpp
    gkfs.io/directory_validate.cpp
    gkfs.io/unlink.cpp
    gkfs.io/access.cpp
    gkfs.io/statfs.cpp
    gkfs.io/dup_validate.cpp
    gkfs.io/syscall_coverage.cpp
    gkfs.io/rename.cpp
)

include(load_nlohmann_json)

target_include_directories(gkfs.io PRIVATE
    ${BOOST_PREPROCESSOR_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/gkfs.io>
)
target_link_libraries(gkfs.io
    nlohmann_json::nlohmann_json
    fmt::fmt
    CLI11::CLI11
    std::filesystem
)

if (GKFS_INSTALL_TESTS)
    install(TARGETS gkfs.io
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif ()
