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

#include <catch2/catch.hpp>
#include <client/path.hpp>
#include <client/preload_context.hpp>

// mock CTX
class text_context {
	std::string mntdir = "/tmp/gkfs_mount";
	const std::string& mountdir(){
		return mntdir;
	}
};
#define CTX text_context

SCENARIO(" resolve fn should handle empty path ",
         "[test_path][empty]") {

    GIVEN(" a mount path ") {

		    std::string mntpath = "/tmp/gkfs_mount";

        WHEN(" resolve with empty path ") {
            THEN(" / should be returned ") {
                REQUIRE(gkfs::path::resolve_new("").second ==  "/");
            }
        }
    }
}

// TODO check pair.first is true
SCENARIO(" resolve fn should handle internal paths ",
         "[test_path][external paths]") {

    GIVEN(" a mount path ") {

		    std::string mntpath = "/tmp/gkfs_mount";

        WHEN(" resolve with relative path ") {
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/foo/../../tmp/./gkfs_mount/bar/./").second ==  "/bar/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/tmp/../tmp/gkfs_mount/bar/./").second ==  "/bar/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/tmp/./gkfs_mount/bar/./").second ==  "/bar/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/tmp/gkfs_mount/bar/./").second ==  "/bar/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/foo/../.././tmp/gkfs_mount/").second ==  "/");
            }
        }
    }
}

// TODO check pair.first is false
SCENARIO(" resolve fn should handle external paths ",
         "[test_path][external paths]") {

    GIVEN(" a mount path ") {

		    std::string mntpath = "/tmp/gkfs_mount";

        WHEN(" resolve with relative path ") {
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/foo/../ar/.").second ==  "/home/bar");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/foo/../bar/./").second ==  "/home/bar/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/foo/../bar/../").second ==  "/home/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/foo/../../").second ==  "/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/foo/./bar/../").second ==  "/home/foo/");
            }
            THEN(" iwas ") {
                REQUIRE(gkfs::path::resolve_new("/home/./../tmp/").second ==  "/tmp/");
            }
        }
    }
}
