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
#include <client/preload.hpp>

#define TEST_PAIR(p, s, s2)                                                    \
    REQUIRE(p.first == s);                                                     \
    REQUIRE(p.second == s2);

SCENARIO(" resolve fn should handle empty path ", "[test_path][empty]") {

    GIVEN(" a mount path ") {

        CTX->mountdir("/home/foo/tmp/gkfs_mount");
        CTX->cwd("/home/foo");

        WHEN(" resolve with empty path ") {
            THEN("") {
                TEST_PAIR(gkfs::path::resolve_new("./tmp/gkfs_mount"), true,
                          "/");
                TEST_PAIR(gkfs::path::resolve_new(""), false, "/");
                TEST_PAIR(gkfs::path::resolve_new("///"), false, "/");
                TEST_PAIR(gkfs::path::resolve_new("tmp/../gkfs_mount"), false,
                          "/home/foo/gkfs_mount");
            }
        }
    }
}

SCENARIO(" resolve fn should handle internal paths ",
         "[test_path][external paths]") {

    GIVEN(" a mount path ") {

        CTX->mountdir("/home/foo/tmp/gkfs_mount");
        CTX->cwd("/home/foo");

        WHEN(" resolve with absolute path ") {
            THEN(" ") {
                TEST_PAIR(gkfs::path::resolve_new(
                                  "/home/foo/../foo//tmp/./gkfs_mount/bar/./"),
                          true, "/bar");
                TEST_PAIR(gkfs::path::resolve_new(
                                  "/home/foo/tmp/../tmp/gkfs_mount/bar/./"),
                          true, "/bar");
                TEST_PAIR(gkfs::path::resolve_new(
                                  "/home/foo/tmp/./gkfs_mount/bar/./"),
                          true, "/bar");
                TEST_PAIR(gkfs::path::resolve_new(
                                  "/home/foo/tmp/gkfs_mount/bar/./"),
                          true, "/bar");
                TEST_PAIR(gkfs::path::resolve_new(
                                  "/home/foo/../../home/foo/./tmp/gkfs_mount/"),
                          true, "/");
            }
        }

        WHEN(" resolve with relative path ") {
            THEN(" ") {
                TEST_PAIR(gkfs::path::resolve_new(
                                  "./sme/blub/../../tmp/./gkfs_mount/bar/./"),
                          true, "/bar");
                TEST_PAIR(gkfs::path::resolve_new(
                                  "./tmp/../tmp/gkfs_mount/bar/./"),
                          true, "/bar");
            }
        }
    }
}

SCENARIO(" resolve fn should handle external paths ",
         "[test_path][external paths]") {

    GIVEN(" a mount path ") {

        CTX->mountdir("/home/foo/tmp/gkfs_mount");
        CTX->cwd("/home/foo");

        WHEN(" resolve with absolute path ") {
            THEN(" ") {
                TEST_PAIR(gkfs::path::resolve_new("/home/foo/../bar/."), false,
                          "/home/bar");
                TEST_PAIR(gkfs::path::resolve_new("/home/foo/../bar/./"), false,
                          "/home/bar");
                TEST_PAIR(gkfs::path::resolve_new("/home/foo/../bar/../"),
                          false, "/home");
                TEST_PAIR(gkfs::path::resolve_new("/home/foo/../../"), false,
                          "/");
                TEST_PAIR(gkfs::path::resolve_new("/home/foo/./bar/../"), false,
                          "/home/foo");
                TEST_PAIR(gkfs::path::resolve_new("/home/./../tmp/"), false,
                          "/tmp");
                TEST_PAIR(gkfs::path::resolve_new(
                                  "/home/./../tmp/gkfs_mount/../"),
                          false, "/tmp");
                TEST_PAIR(gkfs::path::resolve_new("/home/random/device"), false,
                          "/home/random/device");
            }
        }

        WHEN(" resolve with relative path ") {
            THEN(" ") {
                TEST_PAIR(gkfs::path::resolve_new(
                                  "./sme/blub/../tmp/./gkfs_mount/bar/./"),
                          false, "/home/foo/sme/tmp/gkfs_mount/bar");
                TEST_PAIR(gkfs::path::resolve_new("./tmp//bar/./"), false,
                          "/home/foo/tmp/bar");
                TEST_PAIR(gkfs::path::resolve_new("../../../.."), false, "/");
                TEST_PAIR(gkfs::path::resolve_new("../../../../foo"), false, "/foo");
            }
        }
    }
}
