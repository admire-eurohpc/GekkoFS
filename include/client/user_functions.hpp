/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS' POSIX interface.

  GekkoFS' POSIX interface is free software: you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  GekkoFS' POSIX interface is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with GekkoFS' POSIX interface.  If not, see
  <https://www.gnu.org/licenses/>.

  SPDX-License-Identifier: LGPL-3.0-or-later
*/

#ifndef GEKKOFS_USER_FUNCTIONS_HPP
#define GEKKOFS_USER_FUNCTIONS_HPP
#include <string>
#include <cstdint>
#include <vector>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
}

struct linux_dirent64;

namespace gkfs::syscall {

int
gkfs_open(const std::string& path, mode_t mode, int flags);

int
gkfs_create(const std::string& path, mode_t mode);

int
gkfs_remove(const std::string& path);

ssize_t
gkfs_write(int fd, const void* buf, size_t count);

ssize_t
gkfs_read(int fd, void* buf, size_t count);

int
gkfs_close(unsigned int fd);

off64_t
gkfs_lseek(unsigned int fd, off64_t offset, unsigned int whence);

ssize_t
gkfs_pwrite(int fd, const void* buf, size_t count, off64_t offset);

ssize_t
gkfs_pread(int fd, void* buf, size_t count, off64_t offset);

int
gkfs_stat(const std::string& path, struct stat* buf, bool follow_links = true);

int
gkfs_remove(const std::string& path);

std::vector<std::string>
gkfs_get_file_list(const std::string& path);
} // namespace gkfs::syscall


extern "C" int
gkfs_init();

extern "C" int
gkfs_end();

#endif // GEKKOFS_USER_FUNCTIONS_HPP
