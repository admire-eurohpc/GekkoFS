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
/**
 * @brief This is main source for copy manager
 * @internal
 * This file includes all functions which sends and recieves request from HSM
 * It can also check the status of transfer from cargo
 * @endinternal
 */
#include <iostream>
#include <cstring>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <cctype>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <dirent.h>
#include <cerrno>
#include <getopt.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <vector>
#include <cargo.hpp>
#include <time.h>
#include <libcfs/util/string.h>
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


/// intialze o_archive_cnt
struct options {
    int o_archive_cnt;
    int o_archive_id[LL_HSM_MAX_ARCHIVE];
    int o_report_int;
    char* o_event_fifo;
    char* o_mnt;
    int o_mnt_fd;
    char* o_hsm_root;
};

// Everything else is zeroed
struct options opt = {
    .o_report_int = REPORT_INTERVAL_DEFAULT,
};

// HSM copytool private will hold an open FD on the Lustre mount point for us.
// Additionally open one on the archive FS root to make sure it doesn't drop out
// from under us (and remind the admin to shut down the copytool before unmounting).
static int arc_fd = -1;



static char fs_name[MAX_OBD_NAME + 1];

static struct hsm_copytool_private* ctdata;

static inline double wrapper_ct_now(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec + 0.000001 * tv.tv_usec;
}



static volatile bool terminate_requested = false;

struct wrapper_ct_th_data {
    long hal_flags;
    struct hsm_action_item* hai;
};

static void handler(int signal) {
    psignal(signal, "exiting");

    // Clean up upon interrupt
    llapi_hsm_copytool_unregister(&ctdata);

    // Remove fifo upon signal as during normal/error exit
    if (opt.o_event_fifo != nullptr) {
        llapi_hsm_unregister_event_fifo(opt.o_event_fifo);
    }

    std::_Exit(1);
}
/// @brief checking the lustre path
/// @param buf buffer
/// @param sz size
/// @param mnt mount point
/// @param fid file id
/// @return 
static int wrapper_ct_path_lustre(char* buf, int sz, const char* mnt, const struct lu_fid* fid) {
    return snprintf(buf, sz, "%s/%s/fid/" DFID_NOBRACE, mnt, dot_lustre_name, PFID(fid));
}

/// @brief 
/// @param buf 
/// @param sz 
/// @param archive_dir 
/// @param fid 
/// @return 
static int wrapper_ct_path_archive(char* buf, int sz, const char* archive_dir, const struct lu_fid* fid) {
    return snprintf(buf, sz, "%s/%04x/%04x/%04x/%04x/%04x/%04x/" DFID_NOBRACE,
                    archive_dir,
                    (fid)->f_oid & 0xFFFF,
                    (fid)->f_oid >> 16 & 0xFFFF,
                    (unsigned int)((fid)->f_seq & 0xFFFF),
                    (unsigned int)((fid)->f_seq >> 16 & 0xFFFF),
                    (unsigned int)((fid)->f_seq >> 32 & 0xFFFF),
                    (unsigned int)((fid)->f_seq >> 48 & 0xFFFF),
                    PFID(fid));
}
static int wrapper_ct_begin(struct hsm_copyaction_private **phcp, const struct hsm_action_item* hai, int mdt_index, int open_flags) {
    char src[PATH_MAX];
    int rc;

    rc = llapi_hsm_action_begin(phcp, ctdata, hai, mdt_index, open_flags, false);
    if (rc < 0) {
        ct_path_lustre(src, sizeof(src), opt.o_mnt, &hai->hai_fid);
        std::cerr << "llapi_hsm_action_begin() on " << rc << " failed '" << src << "'" << std::endl;

    }

    return rc;
}

static int wrapper_ct_fini(struct hsm_copyaction_private **phcp, const struct hsm_action_item *hai, int hp_flags, int ct_rc) {
    struct hsm_copyaction_private *hcp;
    char lstr[PATH_MAX];
    int rc;


    wrapper_ct_path_lustre(lstr, sizeof(lstr), opt.o_mnt, &hai->hai_fid);

    if (phcp == nullptr || *phcp == nullptr) {
        rc = llapi_hsm_action_begin(&hcp, ctdata, hai, -1, 0, true);
        if (rc < 0) {
            std::cerr << "llapi_hsm_action_begin() on " << rc << " failed '" << lstr << "'" << std::endl;
            
            return rc;
        }
        phcp = &hcp;
    }

    rc = llapi_hsm_action_end(phcp, &hai->hai_extent, hp_flags, abs(ct_rc));
    if (rc == -ECANCELED)
        std::cerr << "Canceled" << std::endl;
    else if (rc < 0)
        std::cerr << "llapi_hsm_action_begin() on " << rc << " failed '" << lstr << "'" << std::endl;

    return rc;
}

static int wrapper_ct_restore(const struct hsm_action_item *hai, const long hal_flags) {
    struct hsm_copyaction_private *hcp = nullptr;
    char src[PATH_MAX];
    char dst[PATH_MAX];
    int rc;
    int hp_flags = 0;
    int mdt_index = -1;
    int open_flags = 0;
    struct lu_fid dfid;
    uint64_t length = 0;
    struct hsm_extent he;
    std::vector<dataset> sources, targets;
    cargo::server srv(get_initialized_server());

    wrapper_ct_path_archive(src, sizeof(src), opt.o_hsm_root, &hai->hai_fid);


    rc = wrapper_ct_begin(&hcp, hai, mdt_index, open_flags);
    if (rc < 0)
        goto fini;

    rc = llapi_hsm_action_get_dfid(hcp, &dfid);
    if (rc < 0) {
        
        goto fini;
    }

    snprintf(dst, sizeof(dst), "{VOLATILE}=" DFID, PFID(&dfid));

    he.offset = offset;
	he.length = 0; 

    sources.emplace_back(src, dataset::type::posix);
    targets.emplace_back(dst, dataset::type::parallel);

    

    ///start cargo
    const auto transfer_result = cargo::transfer_datasets(srv, source, target);
     
    //// report from
    // wait for transfer
    auto s = transfer_result.wait();
    while (s.state()== cargo::transfer_state::running)
    {
        now = time(NULL);
		if (now >= last_report_time + opt.o_report_int) {
			last_report_time = now;
			he.length = offset - he.offset;
			rc = llapi_hsm_action_progress(hcp, &he, length, 0);
			he.offset = offset;
            length = length + 5;
		}
    }
    

    // Transfer was successful
    if (s.state()== cargo::transfer_state::completed && s.error() == cargo::error_code::success) {

        return;
    }
    else
    {   
        std::cerr << "Error transferring datasets: " << s.error() << '\n';
            return;
        }   

    //report

fini:
    rc = wrapper_ct_fini(&hcp, hai, hp_flags, rc);

    return rc;
}

static int wrapper_ct_archive(const struct hsm_action_item *hai, const long hal_flags) {
    struct hsm_copyaction_private *hcp = nullptr;
    struct hsm_extent he;
    char src[PATH_MAX];
    char dst[PATH_MAX] = "";
    int rc;
    int rcf = 0;
    int hp_flags = 0;
    int open_flags;
    int src_fd = -1;
    int dst_fd = -1;
    uint64_t length = 0;
    std::vector<dataset> sources, targets;
    cargo::server srv(get_initialized_server());
    

    rc = ct_begin(&hcp, hai);

    // We fill the archive like so:
    // Source = data FID
    // Destination = Lustre FID
    wrapper_ct_path_lustre(src, sizeof(src), opt.mnt, &hai->hai_dfid);
    wrapper_ct_path_archive(dst, sizeof(dst), opt.hsm_root, &hai->hai_fid);
    he.offset = offset;
	he.length = 0; 

    sources.emplace_back(src, dataset::type::parallel);
    targets.emplace_back(dst, dataset::type::posix);
    

    /////transfer with cargo
    const auto transfer_result = cargo::transfer_datasets(srv, source, target);
     
    //// report from
    // wait for transfer
    auto s = transfer_result.wait();
    while (s.state()== cargo::transfer_state::running)
    {
        now = time(NULL);
		if (now >= last_report_time + opt.o_report_int) {
			last_report_time = now;
			he.length = offset - he.offset;
			rc = llapi_hsm_action_progress(hcp, &he, length, 0);
			he.offset = offset;
            length = length + 5;
		}
    }
    

    // Transfer was successful
    if (s.state()== cargo::transfer_state::completed && s.error() == cargo::error_code::success) {

        return;
    }
    else
    {   
        std::cerr << "Error transferring datasets: " << s.error() << '\n';
            return;
        }   

    rc = wrapper_ct_fini(&hcp, hai, hp_flags, rcf);

    return rc;
}


static int wrapper_ct_process_item(struct hsm_action_item* hai, const long hal_flags) {
    int rc = 0;

    if (opt.verbose >= LLAPI_MSG_INFO || opt.dry_run) {
        // Print the original path
        char fid[128];
        char path[PATH_MAX];
        long long recno = -1;
        int linkno = 0;

        snprintf(fid, sizeof(fid), DFID, PFID(&hai->hai_fid));
    
        rc = llapi_fid2path(opt.mnt, fid, path,
                            sizeof(path), &recno, &linkno);
    
        
    }

    switch (hai->hai_action) {
        // Set err_major, minor inside these functions
        case HSMA_ARCHIVE:
            rc = wrapper_ct_archive(hai, hal_flags);
            break;
        case HSMA_RESTORE:
            rc = wrapper_ct_restore(hai, hal_flags);
            break;
        default:
            rc = -EINVAL;
            wrapper_ct_fini(nullptr, hai, 0, rc);
    }

    return 0;
}

static void* wrapper_ct_thread(void* arg) {
    struct wrapper_ct_th_data* cttd = static_cast<struct wrapper_ct_th_data*>(arg);
    int rc = wrapper_ct_process_item(cttd->hai, cttd->hal_flags);

    // Clean up allocated memory
    free(cttd->hai);
    free(cttd);

    // Use intptr_t to safely cast int to void pointer
    return reinterpret_cast<void*>(static_cast<intptr_t>(rc));
}

static int wrapper_ct_process_item_async(const struct hsm_action_item* hai, long hal_flags) {
    pthread_attr_t attr;
    pthread_t thread;
    struct wrapper_ct_th_data* data;
    int rc;

    data = static_cast<struct wrapper_ct_th_data*>(std::malloc(sizeof(*data)));
    if (data == nullptr) {
        return -ENOMEM;
    }

    data->hai = static_cast<struct hsm_action_item*>(std::malloc(hai->hai_len));
    if (data->hai == nullptr) {
        std::free(data);
        return -ENOMEM;
    }

    std::memcpy(data->hai, hai, hai->hai_len);
    data->hal_flags = hal_flags;

    rc = pthread_attr_init(&attr);
    if (rc != 0) {
        std::cerr << "pthread_attr_init failed for '" << opt.o_mnt << "' service: " << std::strerror(rc) << std::endl;
        std::free(data->hai);
        std::free(data);
        return -rc;
    }

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    rc = pthread_create(&thread, &attr, wrapper_ct_thread, data);
    if (rc != 0) {
        std::cerr << "Cannot create thread for '" << opt.o_mnt << "' service: " << std::strerror(rc) << std::endl;
    }

    pthread_attr_destroy(&attr);
    return 0;
}

static int wrapper_ct_run() {
    struct sigaction cleanup_sigaction;
    int rc = 0;



    setbuf(stdout, nullptr);

    if (opt.o_event_fifo != nullptr) {
        rc = llapi_hsm_register_event_fifo(opt.o_event_fifo);
        if (rc < 0) {
            std::cerr << "Failed to register event fifo: " << strerror(-rc) << std::endl;
            return rc;
        }
    }

    rc = llapi_hsm_copytool_register(&ctdata, opt.o_mnt, opt.o_archive_cnt, opt.o_archive_id, 0);
    if (rc < 0) {
        std::cerr << "Cannot start copytool interface: " << strerror(-rc) << std::endl;
        return rc;
    }

    memset(&cleanup_sigaction, 0, sizeof(cleanup_sigaction));
    cleanup_sigaction.sa_handler = handler;
    sigemptyset(&cleanup_sigaction.sa_mask);
    sigaction(SIGINT, &cleanup_sigaction, nullptr);
    sigaction(SIGTERM, &cleanup_sigaction, nullptr);

    while (!terminate_requested) {
        struct hsm_action_list* hal;
        struct hsm_action_item* hai;
        int msgsize;
        int i = 0;

        //std::cout << "waiting for message from kernel" << std::endl;

        rc = llapi_hsm_copytool_recv(ctdata, &hal, &msgsize);
        if (rc == -ESHUTDOWN) {
            //std::cout << "shutting down" << std::endl;
            break;
        } else if (rc < 0) {
            std::cerr << "Cannot receive action list: " << strerror(-rc) << std::endl;
            // Handle the error or exit the loop.
            break;
        }

        /*std::cout << "copytool fs=" << hal->hal_fsname
                  << " archive#=" << hal->hal_archive_id
                  << " item_count=" << hal->hal_count << std::endl;*/

        if (strcmp(hal->hal_fsname, fs_name) != 0) {
            rc = -EINVAL;
            std::cerr << "'" << hal->hal_fsname << "' invalid fs name, expecting: " << fs_name << std::endl;
            // Handle the error or exit the loop.
            break;
        }

        hai = hai_first(hal);
        while (++i <= hal->hal_count) {
            if ((char*)hai - (char*)hal > msgsize) {
                rc = -EPROTO;
                std::cerr << "'" << opt.o_mnt << "' item " << i << " past end of message!" << std::endl;
                // Handle the error or exit the loop.
                break;
            }
            rc = wrapper_ct_process_item_async(hai, hal->hal_flags);
            if (rc < 0) {
                std::cerr << "'" << opt.o_mnt << "' item " << i << " process" << std::endl;
            }
            hai = hai_next(hai);
        }

        if (terminate_requested) {
            break;
        }
    }

    llapi_hsm_copytool_unregister(&ctdata);
    if (opt.o_event_fifo != nullptr) {
        llapi_hsm_unregister_event_fifo(opt.o_event_fifo);
    }

    return rc;
}



static int wrapper_ct_setup() {
    int rc;


    int arc_fd = open(opt.o_hsm_root, O_RDONLY);
    if (arc_fd < 0) {
        rc = -errno;
        std::cerr << "Cannot open archive at '" << opt.o_hsm_root << "': " << strerror(errno) << std::endl;
        return rc;
    }

    rc = llapi_search_fsname(opt.o_mnt, fs_name);
    if (rc < 0) {
        std::cerr << "Cannot find a Lustre filesystem mounted at '" << opt.o_mnt << "': " << strerror(errno) << std::endl;
        return rc;
    }

    opt.o_mnt_fd = open(opt.o_mnt, O_RDONLY);
    if (opt.o_mnt_fd < 0) {
        rc = -errno;
        std::cerr << "Cannot open mount point at '" << opt.o_mnt << "': " << strerror(errno) << std::endl;
        return rc;
    }

    return rc;
}

static void wrapper_ct_shutdown()
{
     
    int rc;

    if (opt.o_mnt_fd >= 0) {
        rc = close(opt.o_mnt_fd);
        if (rc < 0) {
            rc = -errno;
            std::cerr << "Cannot close mount point: " << strerror(errno) << std::endl;
            // Handle the error or exit the program.
        }
    }

    if (arc_fd >= 0) {
        rc = close(arc_fd);
        if (rc < 0) {
            rc = -errno;
            std::cerr << "Cannot close archive root directory: " << strerror(errno) << std::endl;
            // Handle the error or exit the program.
        }
    }

    if (rc == 0) {
        std::cout << "Shutdown completed successfully." << std::endl;
    }


}
}







