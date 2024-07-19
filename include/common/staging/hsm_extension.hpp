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

#pragma once

#include <cstring>
#include <cerrno>
#include <iostream>
#include <ctime>
#include <vector>
#include <cstdlib>
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <lustre/lustre_user.h>
#include <linux/lustre/lustre_fid.h>
#include <lustre/lustreapi.h>

namespace hsmextension {

    constexpr int REPORT_INTERVAL_DEFAULT = 30;
    constexpr mode_t DIR_PERM = S_IRWXU;
    constexpr mode_t FILE_PERM = (S_IRUSR | S_IWUSR);
    constexpr size_t ONE_MB = 0x100000;

    #ifndef NSEC_PER_SEC
    # define NSEC_PER_SEC 1000000000UL
    #endif

    struct options {
        int o_archive_cnt;
        int o_archive_id[LL_HSM_MAX_ARCHIVE];
        int o_report_int;
        const char* o_event_fifo;
        const char* o_mnt;
        int o_mnt_fd;
        const char* o_hsm_root;
    };

    extern struct options opt;

    struct wrapper_ct_th_data {
        long hal_flags;
        struct hsm_action_item* hai;
    };

    extern volatile bool terminate_requested;

    void handler(int signal);

    int wrapper_ct_path_lustre(char* buf, int sz, const char* mnt, const struct lu_fid* fid);

    int wrapper_ct_path_archive(char* buf, int sz, const char* archive_dir, const struct lu_fid* fid);

    int wrapper_ct_begin(struct hsm_copyaction_private **phcp, const struct hsm_action_item* hai, int mdt_index, int open_flags);

    int wrapper_ct_fini(struct hsm_copyaction_private **phcp, const struct hsm_action_item *hai, int hp_flags, int ct_rc);

    int wrapper_ct_restore(const struct hsm_action_item *hai, const long hal_flags);

    int wrapper_ct_archive(const struct hsm_action_item *hai, const long hal_flags);

    int wrapper_ct_process_item(struct hsm_action_item* hai, const long hal_flags);

    void* wrapper_ct_thread(void* arg);

    int wrapper_ct_process_item_async(const struct hsm_action_item* hai, long hal_flags);

    int wrapper_ct_run();

    int wrapper_ct_setup();

    void wrapper_ct_shutdown();

} // namespace hsmextension
