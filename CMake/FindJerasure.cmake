################################################################################
# Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany          #
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

#
# - Try to find Jerasure library
# This will define
# Jerasure_FOUND
# Jerasure_INCLUDE_DIR
# Jerasure_LIBRARIES
#

find_path(Jerasure_INCLUDE_DIR
    NAMES jerasure.h
    )

find_library(Jerasure_LIBRARY
    NAMES Jerasure
    )

set(Jerasure_LIBRARIES ${Jerasure_LIBRARY})
set(Jerasure_INCLUDE_DIRS ${Jerasure_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Jerasure
    DEFAULT_MSG Jerasure_LIBRARY Jerasure_INCLUDE_DIR
    )

mark_as_advanced(
    Jerasure_LIBRARY
    Jerasure_INCLUDE_DIR
)