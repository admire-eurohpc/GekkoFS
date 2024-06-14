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

set (CMAKE_CXX_STANDARD 17)

add_executable(sfind sfind.cpp)
set_property(TARGET sfind PROPERTY POSITION_INDEPENDENT_CODE ON)
if(GKFS_INSTALL_TESTS)
    install(TARGETS sfind
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

find_package(MPI)
if (MPI_FOUND)
    message(STATUS "[gekkofs] MPI was found. Building gfind example")
    add_executable(gfind gfind.cpp)
    set_property(TARGET gfind PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(gfind
        PUBLIC
        MPI::MPI_CXX
    )
    if(GKFS_INSTALL_TESTS)
        install(TARGETS gfind
            DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
endif()
