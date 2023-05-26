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
# - Try to find GF_complete library
# This will define
# GF_complete_FOUND
# GF_complete_INCLUDE_DIR
# GF_complete_LIBRARIES
#

find_path(GF_complete_INCLUDE_DIR
    NAMES gf_complete.h
    )

find_library(GF_complete_LIBRARY
    NAMES gf_complete
    )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( GF_complete 
    DEFAULT_MSG 
    GF_complete_INCLUDE_DIR
    GF_complete_LIBRARY
)

if(GF_complete_FOUND)
    set(GF_complete_INCLUDE_DIRS ${GF_complete_INCLUDE_DIR})
    set(GF_complete_LIBRARIES ${GF_complete_LIBRARY})

    if(NOT TARGET GF_complete::GF_complete)
        add_library(GF_complete::GF_complete UNKNOWN IMPORTED)
        set_target_properties(GF_complete::GF_complete PROPERTIES
            IMPORTED_LOCATION "${GF_complete_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${GF_complete_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(
    GF_complete_INCLUDE_DIR
    GF_complete_LIBRARY
)
