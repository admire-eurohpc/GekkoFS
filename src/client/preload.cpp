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

#include <client/preload.hpp>
#include <client/path.hpp>
#include <client/logging.hpp>
#include <client/rpc/forward_management.hpp>
#include <client/preload_util.hpp>
#include <client/intercept.hpp>
#include <client/cache.hpp>

#include <common/rpc/distributor.hpp>
#include <common/common_defs.hpp>
#ifdef GKFS_ENABLE_CLIENT_METRICS
#include <common/msgpack_util.hpp>
#endif

#include <ctime>
#include <cstdlib>
#include <fstream>

#include <hermes.hpp>


using namespace std;

std::unique_ptr<hermes::async_engine> ld_network_service; // extern variable
std::unique_ptr<hermes::async_engine> ld_proxy_service;   // extern variable

namespace {

// FORWARDING
pthread_t mapper;
bool forwarding_running;

pthread_mutex_t remap_mutex;
pthread_cond_t remap_signal;
// END FORWARDING

inline void
exit_error_msg(int errcode, const string& msg) {

    LOG_ERROR("{}", msg);
    gkfs::log::logger::log_message(stderr, "{}\n", msg);

    // if we don't disable interception before calling ::exit()
    // syscall hooks may find an inconsistent in shared state
    // (e.g. the logger) and thus, crash
    if(CTX->interception_enabled()) {
        gkfs::preload::stop_interception();
        CTX->disable_interception();
    }
    ::exit(errcode);
}

/**
 * Initializes the Hermes client for a given transport prefix
 * @return true if successfully initialized; false otherwise
 */
bool
init_hermes_client() {

    try {

        hermes::engine_options opts{};

        if(CTX->auto_sm())
            opts |= hermes::use_auto_sm;
        if(gkfs::rpc::protocol::ofi_psm2 == CTX->rpc_protocol()) {
            opts |= hermes::force_no_block_progress;
        }

        opts |= hermes::process_may_fork;

        ld_network_service = std::make_unique<hermes::async_engine>(
                hermes::get_transport_type(CTX->rpc_protocol()), opts);
        ld_network_service->run();
    } catch(const std::exception& ex) {
        fmt::print(stderr, "Failed to initialize Hermes RPC client {}\n",
                   ex.what());
        return false;
    }
    if(CTX->use_proxy()) {
        try {
            LOG(INFO, "Initializing IPC proxy subsystem...");
            hermes::engine_options opts{};
            ld_proxy_service = std::make_unique<hermes::async_engine>(
                    hermes::get_transport_type("na+sm"), opts, "", false, 1);
            ld_proxy_service->run();
        } catch(const std::exception& ex) {
            fmt::print(stderr,
                       "Failed to initialize Hermes IPC client for proxy {}\n",
                       ex.what());
            return false;
        }
    }

    return true;
}

void*
forwarding_mapper(void* p) {
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += 10; // 10 seconds

    int previous = -1;

    while(forwarding_running) {
        try {
            gkfs::utils::load_forwarding_map();

            if(previous != (int64_t) CTX->fwd_host_id()) {
                LOG(INFO, "{}() Forward to {}", __func__, CTX->fwd_host_id());

                previous = CTX->fwd_host_id();
            }
        } catch(std::exception& e) {
            exit_error_msg(EXIT_FAILURE,
                           fmt::format("Unable set the forwarding host '{}'",
                                       e.what()));
        }

        pthread_mutex_lock(&remap_mutex);
        pthread_cond_timedwait(&remap_signal, &remap_mutex, &timeout);
        pthread_mutex_unlock(&remap_mutex);
    }

    return nullptr;
}

void
init_forwarding_mapper() {
    forwarding_running = true;

    pthread_create(&mapper, NULL, forwarding_mapper, NULL);
}

void
destroy_forwarding_mapper() {
    forwarding_running = false;

    pthread_cond_signal(&remap_signal);

    pthread_join(mapper, NULL);
}

void
log_prog_name() {
    std::string line;
    std::ifstream cmdline("/proc/self/cmdline");
    if(!cmdline.is_open()) {
        LOG(ERROR, "Unable to open cmdline file");
        throw std::runtime_error("Unable to open cmdline file");
    }
    if(!getline(cmdline, line)) {
        throw std::runtime_error("Unable to read cmdline file");
    }
    std::replace(line.begin(), line.end(), '\0', ' ');
    line.erase(line.length() - 1, line.length());
    LOG(INFO, "Process cmdline: '{}'", line);
    cmdline.close();
}

} // namespace

namespace gkfs::preload {

/**
 * This function is only called in the preload constructor and initializes
 * the file system client
 */
void
init_environment() {

    vector<pair<string, string>> hosts{};
    try {
        LOG(INFO, "Loading peer addresses...");
        hosts = gkfs::utils::read_hosts_file();
    } catch(const std::exception& e) {
        exit_error_msg(EXIT_FAILURE,
                       "Failed to load hosts addresses: "s + e.what());
    }

    LOG(INFO, "Checking for GKFS Proxy");
    gkfs::utils::check_for_proxy();

    // initialize Hermes interface to Mercury
    LOG(INFO, "Initializing RPC subsystem...");

    if(!init_hermes_client()) {
        exit_error_msg(EXIT_FAILURE, "Unable to initialize RPC subsystem");
    }

    try {
        gkfs::utils::connect_to_hosts(hosts);
        if(CTX->use_proxy()) {
            LOG(INFO, "Connecting to proxy...");
            gkfs::utils::lookup_proxy_addr();
        }
    } catch(const std::exception& e) {
        exit_error_msg(EXIT_FAILURE,
                       "Failed to connect to hosts: "s + e.what());
    }

    /* Setup distributor */
    auto forwarding_map_file = gkfs::env::get_var(
            gkfs::env::FORWARDING_MAP_FILE, gkfs::config::forwarding_file_path);

    if(!forwarding_map_file.empty()) {
        try {
            gkfs::utils::load_forwarding_map();

            LOG(INFO, "{}() Forward to {}", __func__, CTX->fwd_host_id());
        } catch(std::exception& e) {
            exit_error_msg(EXIT_FAILURE,
                           fmt::format("Unable set the forwarding host '{}'",
                                       e.what()));
        }

        auto forwarder_dist = std::make_shared<gkfs::rpc::ForwarderDistributor>(
                CTX->fwd_host_id(), CTX->hosts().size());
        CTX->distributor(forwarder_dist);
    } else {

#ifdef GKFS_USE_GUIDED_DISTRIBUTION
        auto distributor = std::make_shared<gkfs::rpc::GuidedDistributor>(
                CTX->local_host_id(), CTX->hosts().size());
#else
        auto distributor = std::make_shared<gkfs::rpc::SimpleHashDistributor>(
                CTX->local_host_id(), CTX->hosts().size());
#endif
        CTX->distributor(distributor);
    }

    auto use_dcache = gkfs::env::get_var(gkfs::env::cache::DENTRY,
                                         gkfs::config::cache::use_dentry_cache
                                                 ? "ON"
                                                 : "OFF") == "ON";
    if(use_dcache) {
        try {
            LOG(INFO, "Initializing dentry caching...");
            auto dentry_cache =
                    std::make_shared<gkfs::cache::dir::DentryCache>();
            CTX->dentry_cache(dentry_cache);
            LOG(INFO, "dentry caching enabled.");
            CTX->use_dentry_cache(true);
        } catch(const std::exception& e) {
            exit_error_msg(EXIT_FAILURE,
                           "Failed to initialize dentry cache: "s + e.what());
        }
    } else {
        if(gkfs::env::var_is_set(gkfs::env::cache::DENTRY)) {
            LOG(INFO, "Dentry cache is disabled by environment variable.");
        } else {
            LOG(INFO, "Dentry cache is disabled by configuration.");
        }
    }

    auto use_write_size_cache =
            gkfs::env::get_var(gkfs::env::cache::WRITE_SIZE,
                               gkfs::config::cache::use_write_size_cache
                                       ? "ON"
                                       : "OFF") == "ON";
    if(use_write_size_cache) {
        try {
            LOG(INFO, "Initializing write size cache...");
            auto write_size_cache =
                    std::make_shared<gkfs::cache::file::WriteSizeCache>();
            CTX->write_size_cache(write_size_cache);
            CTX->write_size_cache()->flush_threshold(gkfs::env::get_var(
                    gkfs::env::cache::WRITE_SIZE_THRESHOLD,
                    gkfs::config::cache::write_size_flush_threshold));
            CTX->use_write_size_cache(true);
            if(CTX->write_size_cache()->flush_threshold() == 0) {
                LOG(WARNING,
                    "Write size cache is enabled but flush threshold is set to 0. Cache is disabled as a result.");
                CTX->use_write_size_cache(false);
            } else {
                LOG(INFO, "Write size cache enabled. Flushing at '{}' writes",
                    CTX->write_size_cache()->flush_threshold());
            }
        } catch(const std::exception& e) {
            exit_error_msg(EXIT_FAILURE,
                           "Failed to initialize write size cache: "s +
                                   e.what());
        }
    } else {
        if(gkfs::env::var_is_set(gkfs::env::cache::WRITE_SIZE)) {
            LOG(INFO, "Write size cache is disabled by environment variable.");
        } else {
            LOG(INFO, "Write size cache is disabled by configuration.");
        }
    }

    LOG(INFO, "Retrieving file system configuration...");

    if(!gkfs::rpc::forward_get_fs_config()) {
        exit_error_msg(
                EXIT_FAILURE,
                "Unable to fetch file system configurations from daemon process through RPC.");
    }
    // Initialize random number generator and seed for replica selection
    // in case of failure, a new replica will be selected
    if(CTX->get_replicas() > 0) {
        srand(time(nullptr));
    }

    LOG(INFO, "Environment initialization successful.");
}

} // namespace gkfs::preload

/**
 * Called initially ONCE when preload library is used with the LD_PRELOAD
 * environment variable
 */
void
init_preload() {
    // The original errno value will be restored after initialization to not
    // leak internal error codes
    auto oerrno = errno;

    CTX->enable_interception();
    gkfs::preload::start_self_interception();

    CTX->init_logging();
    // from here ownwards it is safe to print messages
    LOG(DEBUG, "Logging subsystem initialized");

    // Kernel modules such as ib_uverbs may create fds in kernel space and pass
    // them to user-space processes using ioctl()-like interfaces. if this
    // happens during our internal initialization, there's no way for us to
    // control this creation and the fd will be created in the
    // [0, MAX_USER_FDS) range rather than in our private
    // [MAX_USER_FDS, GKFS_MAX_OPEN_FDS) range.
    // with MAX_USER_FDS = GKFS_MAX_OPEN_FDS - GKFS_MAX_INTERNAL_FDS
    // To prevent this for our internal
    // initialization code, we forcefully occupy the user fd range to force
    // such modules to create fds in our private range.
    CTX->protect_user_fds();

    log_prog_name();
    gkfs::path::init_cwd();

    LOG(DEBUG, "Current working directory: '{}'", CTX->cwd());
    LOG(DEBUG, "Number of replicas : '{}'", CTX->get_replicas());
    gkfs::preload::init_environment();
    CTX->enable_interception();

    CTX->unprotect_user_fds();

    auto forwarding_map_file = gkfs::env::get_var(
            gkfs::env::FORWARDING_MAP_FILE, gkfs::config::forwarding_file_path);
    if(!forwarding_map_file.empty()) {
        init_forwarding_mapper();
    }

    gkfs::preload::start_interception();
    errno = oerrno;
    if(!CTX->init_metrics()) {
        exit_error_msg(EXIT_FAILURE,
                       "Unable to initialize client metrics. Exiting...");
    }
}

/**
 * Called last when preload library is used with the LD_PRELOAD environment
 * variable
 */
void
destroy_preload() {
    auto forwarding_map_file = gkfs::env::get_var(
            gkfs::env::FORWARDING_MAP_FILE, gkfs::config::forwarding_file_path);
    if(!forwarding_map_file.empty()) {
        destroy_forwarding_mapper();
    }
#ifdef GKFS_ENABLE_CLIENT_METRICS
    LOG(INFO, "Flushing final metrics...");
    CTX->write_metrics()->flush_msgpack();
    CTX->read_metrics()->flush_msgpack();
    LOG(INFO, "Metrics flushed. Total flush operations: {}",
        CTX->write_metrics()->flush_count());
#endif
    CTX->clear_hosts();
    LOG(DEBUG, "Peer information deleted");

    if(CTX->use_proxy()) {
        CTX->clear_proxy_host();
        LOG(DEBUG, "Shutting down IPC subsystem");
        ld_proxy_service.reset();
    }
    LOG(DEBUG, "Shutting down RPC subsystem");
    ld_network_service.reset();
    LOG(DEBUG, "RPC subsystem shut down");

    if(CTX->interception_enabled()) {
        gkfs::preload::stop_interception();
        CTX->disable_interception();
        LOG(DEBUG, "Syscall interception stopped");
    }

    LOG(INFO, "All subsystems shut down. Client shutdown complete.");
}


/**
 * @brief External functions to call linking the library
 *
 */
extern "C" int
gkfs_init() {
    CTX->init_logging();

    // from here ownwards it is safe to print messages
    LOG(DEBUG, "Logging subsystem initialized");

    gkfs::preload::init_environment();

    return 0;
}


extern "C" int
gkfs_end() {
    CTX->clear_hosts();
    LOG(DEBUG, "Peer information deleted");

    ld_network_service.reset();
    LOG(DEBUG, "RPC subsystem shut down");

    LOG(INFO, "All subsystems shut down. Client shutdown complete.");

    return 0;
}
