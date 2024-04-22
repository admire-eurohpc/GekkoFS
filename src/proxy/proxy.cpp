/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/
#include <proxy/proxy.hpp>
#include <proxy/util.hpp>
#include <proxy/rpc/rpc_defs.hpp>

#include <common/log_util.hpp>
#include <common/env_util.hpp>
#include <common/rpc/rpc_types.hpp>
#include <common/rpc/distributor.hpp>
#include <common/rpc/rpc_util.hpp>

#include <CLI/CLI.hpp>

#include <iostream>
#include <csignal>
#include <condition_variable>

using namespace std;

static condition_variable shutdown_please;
static mutex mtx;

struct cli_options {
    string hosts_file;
    string proxy_protocol;
    string pid_path;
};

void
register_server_ipcs(margo_instance_id mid) {
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_write,
                   rpc_client_proxy_write_in_t, rpc_data_out_t,
                   proxy_rpc_srv_write)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_read,
                   rpc_client_proxy_read_in_t, rpc_data_out_t,
                   proxy_rpc_srv_read)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_truncate,
                   rpc_client_proxy_trunc_in_t, rpc_err_out_t,
                   proxy_rpc_srv_truncate)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_chunk_stat,
                   rpc_chunk_stat_in_t, rpc_chunk_stat_out_t,
                   proxy_rpc_srv_chunk_stat)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_create, rpc_mk_node_in_t,
                   rpc_err_out_t, proxy_rpc_srv_create)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_stat, rpc_path_only_in_t,
                   rpc_stat_out_t, proxy_rpc_srv_stat)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_remove, rpc_rm_node_in_t,
                   rpc_err_out_t, proxy_rpc_srv_remove)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_update_size,
                   rpc_update_metadentry_size_in_t,
                   rpc_update_metadentry_size_out_t,
                   proxy_rpc_srv_update_metadentry_size)
    MARGO_REGISTER(mid, gkfs::rpc::tag::client_proxy_get_dirents_extended,
                   rpc_proxy_get_dirents_in_t, rpc_get_dirents_out_t,
                   proxy_rpc_srv_get_dirents_extended)
}

void
init_ipc_server() {
    hg_addr_t addr_self;
    hg_size_t addr_self_cstring_sz = 128;
    char addr_self_cstring[128];
    struct hg_init_info hg_options = HG_INIT_INFO_INITIALIZER;
    hg_options.auto_sm = HG_FALSE;
    hg_options.stats = HG_FALSE;

    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto margo_config = fmt::format(
            R"({{ "use_progress_thread" : true, "rpc_thread_count" : {} }})",
            gkfs::config::rpc::proxy_handler_xstreams);
    struct margo_init_info args = {nullptr};
    args.json_config = margo_config.c_str();
    args.hg_init_info = &hg_options;
    auto* mid = margo_init_ext(gkfs::rpc::protocol::na_sm, MARGO_SERVER_MODE,
                               &args);

    //    hg_options.na_class = nullptr;
    //    // Start Margo (this will also initialize Argobots and Mercury
    //    internally) auto mid = margo_init_opt(gkfs::rpc::protocol::na_sm,
    //    MARGO_SERVER_MODE,
    //                              &hg_options, HG_TRUE,
    //                              gkfs::config::rpc::proxy_handler_xstreams);
    if(mid == MARGO_INSTANCE_NULL) {
        throw runtime_error("Failed to initialize the Margo IPC server");
    }
    // Figure out what address this server is listening on (must be freed when
    // finished)
    auto hret = margo_addr_self(mid, &addr_self);
    if(hret != HG_SUCCESS) {
        margo_finalize(mid);
        throw runtime_error("Failed to retrieve server IPC address");
    }
    // Convert the address to a cstring (with \0 terminator).
    hret = margo_addr_to_string(mid, addr_self_cstring, &addr_self_cstring_sz,
                                addr_self);
    if(hret != HG_SUCCESS) {
        margo_addr_free(mid, addr_self);
        margo_finalize(mid);
        throw runtime_error("Failed to convert server IPC address to string");
    }
    margo_addr_free(mid, addr_self);

    std::string addr_self_str(addr_self_cstring);
    PROXY_DATA->server_self_addr(addr_self_str);

    PROXY_DATA->log()->info("{}() Accepting IPCs on address {}", __func__,
                            addr_self_cstring);

    // Put context and class into RPC_data object
    PROXY_DATA->server_ipc_mid(mid);

    // register RPCs
    register_server_ipcs(mid);
}

void
register_client_rpcs(margo_instance_id mid) {
    PROXY_DATA->rpc_client_ids().rpc_write_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::proxy_daemon_write,
                           rpc_proxy_daemon_write_in_t, rpc_data_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_read_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::proxy_daemon_read,
                           rpc_proxy_daemon_read_in_t, rpc_data_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_truncate_id = MARGO_REGISTER(
            mid, gkfs::rpc::tag::truncate, rpc_trunc_in_t, rpc_err_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_chunk_stat_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::get_chunk_stat,
                           rpc_chunk_stat_in_t, rpc_chunk_stat_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_create_id = MARGO_REGISTER(
            mid, gkfs::rpc::tag::create, rpc_mk_node_in_t, rpc_err_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_stat_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::stat, rpc_path_only_in_t,
                           rpc_stat_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_remove_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::remove_metadata,
                           rpc_rm_node_in_t, rpc_rm_metadata_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_remove_data_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::remove_data, rpc_rm_node_in_t,
                           rpc_err_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_update_metadentry_size_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::update_metadentry_size,
                           rpc_update_metadentry_size_in_t,
                           rpc_update_metadentry_size_out_t, NULL);
    PROXY_DATA->rpc_client_ids().rpc_get_dirents_extended_id =
            MARGO_REGISTER(mid, gkfs::rpc::tag::get_dirents_extended,
                           rpc_get_dirents_in_t, rpc_get_dirents_out_t, NULL);
}

void
init_rpc_client(const string& protocol) {
    struct hg_init_info hg_options = HG_INIT_INFO_INITIALIZER;
    hg_options.auto_sm = PROXY_DATA->use_auto_sm() ? HG_TRUE : HG_FALSE;
    hg_options.stats = HG_FALSE;
    if(gkfs::rpc::protocol::ofi_psm2 == protocol.c_str())
        hg_options.na_init_info.progress_mode = NA_NO_BLOCK;
    // Start Margo (this will also initialize Argobots and Mercury internally)
    auto margo_config = fmt::format(
            R"({{ "use_progress_thread" : true, "rpc_thread_count" : {} }})",
            0);
    struct margo_init_info args = {nullptr};
    args.json_config = margo_config.c_str();
    args.hg_init_info = &hg_options;
    auto* mid = margo_init_ext(protocol.c_str(), MARGO_CLIENT_MODE, &args);

    //    hg_options.na_class = nullptr;
    //    if(gkfs::rpc::protocol::ofi_psm2 == protocol.c_str())
    //        hg_options.na_init_info.progress_mode = NA_NO_BLOCK;
    //    // Start Margo (this will also initialize Argobots and Mercury
    //    internally) auto mid = margo_init_opt(protocol.c_str(),
    //    MARGO_CLIENT_MODE, &hg_options,
    //                              HG_TRUE, 0);
    if(mid == MARGO_INSTANCE_NULL) {
        throw runtime_error("Failed to initialize the Margo RPC client");
    }
    PROXY_DATA->log()->info(
            "{}() Margo RPC client initialized with protocol '{}'", __func__,
            protocol);
    PROXY_DATA->log()->info("{}() auto sm is set to '{}' for RPC client.",
                            __func__, PROXY_DATA->use_auto_sm());
    PROXY_DATA->client_rpc_mid(mid);
    register_client_rpcs(mid);
}

void
init_environment(const string& hostfile_path, const string& rpc_protocol) {
    // Check if host file exists before doing anything
    if(!gkfs::util::check_for_hosts_file(hostfile_path))
        throw runtime_error(fmt::format(
                "Host file '{}' does not exist. Exiting ...", hostfile_path));

    // Check if another proxy is already running
    PROXY_DATA->log()->info("{}() Checking for another proxy process...",
                            __func__);
    if(gkfs::util::is_proxy_already_running()) {
        throw runtime_error("Another proxy is already running. Exiting ...");
    }
    PROXY_DATA->log()->info("{}() No other proxy is running. Proceeding ...",
                            __func__);

    vector<pair<string, string>> hosts{};
    try {
        PROXY_DATA->log()->info("{}() Loading daemon hostsfile ...", __func__);
        hosts = gkfs::util::read_hosts_file(hostfile_path);
    } catch(const std::exception& e) {
        auto err_msg =
                fmt::format("Failed to load hosts addresses: {}", e.what());
        throw runtime_error(err_msg);
    }

    // Init IPC server
    PROXY_DATA->log()->info("{}() Initializing IPC server...", __func__);
    try {
        init_ipc_server();
    } catch(const std::exception& e) {
        auto err_msg =
                fmt::format("Failed to initialize IPC server: {}", e.what());
        throw runtime_error(err_msg);
    }

    // Init RPC client
    PROXY_DATA->log()->info("{}() Initializing RPC client...", __func__);
    try {
        init_rpc_client(rpc_protocol);
    } catch(const std::exception& e) {
        auto err_msg =
                fmt::format("Failed to initialize RPC client: {}", e.what());
        throw runtime_error(err_msg);
    }

    // Create PID file
    PROXY_DATA->log()->info("{}() Creating PID file ...", __func__);
    try {
        gkfs::util::create_proxy_pid_file();
    } catch(const std::exception& e) {
        auto err_msg = fmt::format(
                "Unexpected error: '{}' when creating PID file.", e.what());
        throw runtime_error(err_msg);
    }

    // Load hosts from hostfile
    try {
        PROXY_DATA->log()->info(
                "{}() Loading daemon addresses and looking up ...", __func__);
        gkfs::util::connect_to_hosts(hosts);
    } catch(const std::exception& e) {
        auto err_msg =
                fmt::format("Failed to load hosts addresses: '{}'", e.what());
        throw runtime_error(err_msg);
    }

    // Setup SimpleDistributor
    PROXY_DATA->log()->info(
            "{}() Setting up simple hash distributor with local_host_id '{}' #hosts '{}'...",
            __func__, PROXY_DATA->local_host_id(),
            PROXY_DATA->rpc_endpoints().size());
    // TODO this needs to be globally configured because client must have same
    // distribution
    auto simple_hash_dist = std::make_shared<gkfs::rpc::SimpleHashDistributor>(
            PROXY_DATA->local_host_id(), PROXY_DATA->rpc_endpoints().size());
    PROXY_DATA->distributor(simple_hash_dist);

    PROXY_DATA->log()->info("Startup successful. Proxy is ready.");
}

void
destroy_enviroment() {
    PROXY_DATA->log()->info("{}() Closing connections ...", __func__);
    for(auto& endp : PROXY_DATA->rpc_endpoints()) {
        if(margo_addr_free(PROXY_DATA->client_rpc_mid(), endp.second) !=
           HG_SUCCESS) {
            PROXY_DATA->log()->warn(
                    "{}() Unable to free RPC client's address: '{}'.", __func__,
                    endp.first);
        }
    }
    if(PROXY_DATA->server_ipc_mid() != nullptr) {
        PROXY_DATA->log()->info("{}() Finalizing margo IPC server ...",
                                __func__);
        margo_finalize(PROXY_DATA->server_ipc_mid());
    }
    if(PROXY_DATA->client_rpc_mid() != nullptr) {
        PROXY_DATA->log()->info("{}() Finalizing margo RPC client ...",
                                __func__);
        margo_finalize(PROXY_DATA->client_rpc_mid());
    }
    gkfs::util::remove_proxy_pid_file();
}

void
shutdown_handler(int dummy) {
    PROXY_DATA->log()->info("{}() Received signal: '{}'", __func__,
                            strsignal(dummy));
    shutdown_please.notify_all();
}

void
initialize_loggers() {
    std::string path = gkfs::config::log::proxy_log_path;
    // Try to get log path from env variable
    std::string env_path_key = PROXY_ENV_PREFIX;
    env_path_key += "LOG_PATH";
    char* env_path = getenv(env_path_key.c_str());
    if(env_path != nullptr) {
        path = env_path;
    }

    spdlog::level::level_enum level =
            gkfs::log::get_level(gkfs::config::log::proxy_log_level);
    // Try to get log path from env variable
    std::string env_level_key = PROXY_ENV_PREFIX;
    env_level_key += "LOG_LEVEL";
    char* env_level = getenv(env_level_key.c_str());
    if(env_level != nullptr) {
        level = gkfs::log::get_level(env_level);
    }

    auto logger_names = std::vector<std::string>{
            "main",
    };

    gkfs::log::setup(logger_names, level, path);
}

int
main(int argc, const char* argv[]) {

    CLI::App desc{"Allowed options"};
    cli_options opts{};
    // clang-format off
    desc.add_option("--hosts-file,-H", opts.hosts_file,
                    "Path to the shared host file generated by daemons, including all daemon addresses to connect to. (default path './gkfs_hosts.txt')");
    desc.add_option("--proxy-protocol,-p", opts.proxy_protocol,
                    "Used protocol between proxy and daemon communication. Choose between: ofi+sockets, ofi+psm2, ofi+verbs. Default: ofi+sockets");
    desc.add_option("--pid-path,-P", opts.pid_path,
                    "Path to PID file where daemon registers itself for clients. Default: /tmp/gkfs_proxy.pid");
    // clang-format on
    try {
        desc.parse(argc, argv);
    } catch(const CLI::ParseError& e) {
        return desc.exit(e);
    }

    initialize_loggers();
    PROXY_DATA->log(spdlog::get("main"));

    string proxy_protocol = gkfs::rpc::protocol::ofi_sockets;
    if(desc.count("--proxy-protocol")) {
        proxy_protocol = opts.proxy_protocol;
    }
    string hosts_file = gkfs::config::hostfile_path;
    if(desc.count("--hosts-file")) {
        hosts_file = opts.hosts_file;
    }
    if(desc.count("--pid-path")) {
        PROXY_DATA->pid_file_path(opts.pid_path);
    }

    PROXY_DATA->log()->info("{}() Initializing environment", __func__);
    try {
        init_environment(hosts_file, proxy_protocol);
    } catch(const std::exception& e) {
        auto emsg =
                fmt::format("Failed to initialize environment: {}", e.what());
        PROXY_DATA->log()->error(emsg);
        cerr << emsg << endl;
        destroy_enviroment();
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, shutdown_handler);
    signal(SIGTERM, shutdown_handler);
    signal(SIGKILL, shutdown_handler);

    unique_lock<mutex> lk(mtx);
    // Wait for shutdown signal to initiate shutdown protocols
    shutdown_please.wait(lk);
    PROXY_DATA->log()->info("{}() Shutting down...", __func__);
    destroy_enviroment();
    PROXY_DATA->log()->info("{}() Complete. Exiting...", __func__);

    return 0;
}
