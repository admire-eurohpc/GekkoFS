
#include <preload/rpc/ld_rpc_metadentry.hpp>
#include <rpc/rpc_utils.hpp>


using namespace std;

int rpc_send_mk_node(const std::string& path, const mode_t mode) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_mk_node_in_t in{};
    rpc_err_out_t out{};
    hg_return_t ret;
    int err = EUNKNOWN;
    // fill in
    in.path = path.c_str();
    in.mode = mode;

    ld_logger->debug("{}() Creating Mercury handle ...", __func__);
    margo_create_wrap(ipc_mk_node_id, rpc_mk_node_id, path, handle, svr_addr, false);

    ret = HG_OTHER_ERROR;
    ld_logger->debug("{}() About to send RPC ...", __func__);
    for (int i = 0; i < RPC_TRIES; ++i) {
        ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (ret == HG_SUCCESS) {
            break;
        }
    }
    if (ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);

        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        if (out.err != 0)
            errno = out.err;
        else
            err = 0;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out");
    }

    margo_destroy(handle);
    return err;
}

int rpc_send_access(const std::string& path, const int mask) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_access_in_t in{};
    rpc_err_out_t out{};
    hg_return_t ret;
    int err = EUNKNOWN;
    // fill in
    in.path = path.c_str();
    in.mask = mask;
    ld_logger->debug("{}() Creating Mercury handle ...", __func__);
    margo_create_wrap(ipc_access_id, rpc_access_id, path, handle, svr_addr, false);

    ret = HG_OTHER_ERROR;
    ld_logger->debug("{}() About to send RPC ...", __func__);
    for (int i = 0; i < RPC_TRIES; ++i) {
        ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (ret == HG_SUCCESS) {
            break;
        }
    }
    if (ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);
        if (ret != HG_SUCCESS) {
            ld_logger->error("{}() while getting margo output", __func__);
            errno = EIO;
            margo_free_output(handle, &out);
            margo_destroy(handle);
            return -1;
        }
        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        if (out.err != 0)
            errno = out.err;
        else
            err = 0;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out");
    }

    margo_destroy(handle);
    return err;
}

int rpc_send_stat(const std::string& path, string& attr) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_stat_in_t in{};
    rpc_stat_out_t out{};
    hg_return_t ret;
    int err = EUNKNOWN;
    // fill in
    in.path = path.c_str();
    ld_logger->debug("{}() Creating Mercury handle ...", __func__);
    margo_create_wrap(ipc_stat_id, rpc_stat_id, path, handle, svr_addr, false);

    ld_logger->debug("{}() About to send RPC ...", __func__);
    ret = HG_OTHER_ERROR;
    for (int i = 0; i < RPC_TRIES; ++i) {
        ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (ret == HG_SUCCESS) {
            break;
        }
    }
    if (ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);
        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        if (out.err == 0)
            attr = out.db_val;

        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        err = 1;
        ld_logger->warn("{}() timed out", __func__);
    }
    margo_destroy(handle);
    return err;
}

int rpc_send_rm_node(const std::string& path) {
    rpc_rm_node_in_t in{};
    rpc_err_out_t out{};
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    int err = EUNKNOWN;
    // fill in
    in.path = path.c_str();

    ld_logger->debug("{}() Creating Mercury handle ...", __func__);
    margo_create_wrap(ipc_rm_node_id, rpc_rm_node_id, path, handle, svr_addr, false);

    ld_logger->debug("{}() About to send RPC ...", __func__);
    auto ret = HG_OTHER_ERROR;
    for (int i = 0; i < RPC_TRIES; ++i) {
        ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (ret == HG_SUCCESS) {
            break;
        }
    }
    if (ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        ret = margo_get_output(handle, &out);

        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
    }
    margo_destroy(handle);
    return err;
}


int rpc_send_update_metadentry(const string& path, const Metadentry& md, const MetadentryUpdateFlags& md_flags) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_update_metadentry_in_t in{};
    rpc_err_out_t out{};
    int err = EUNKNOWN;
    // fill in
    // add data
    in.path = path.c_str();
    in.size = md_flags.size ? md.size : 0;
    in.nlink = md_flags.link_count ? md.link_count : 0;
    in.gid = md_flags.gid ? md.gid : 0;
    in.uid = md_flags.uid ? md.uid : 0;
    in.blocks = md_flags.blocks ? md.blocks : 0;
    in.inode_no = md_flags.inode_no ? md.inode_no : 0;
    in.atime = md_flags.atime ? md.atime : 0;
    in.mtime = md_flags.mtime ? md.mtime : 0;
    in.ctime = md_flags.ctime ? md.ctime : 0;
    // add data flags
    in.size_flag = bool_to_merc_bool(md_flags.size);
    in.nlink_flag = bool_to_merc_bool(md_flags.link_count);
    in.gid_flag = bool_to_merc_bool(md_flags.gid);
    in.uid_flag = bool_to_merc_bool(md_flags.uid);
    in.block_flag = bool_to_merc_bool(md_flags.blocks);
    in.inode_no_flag = bool_to_merc_bool(md_flags.inode_no);
    in.atime_flag = bool_to_merc_bool(md_flags.atime);
    in.mtime_flag = bool_to_merc_bool(md_flags.mtime);
    in.ctime_flag = bool_to_merc_bool(md_flags.ctime);

    ld_logger->debug("{}() Creating Mercury handle ...", __func__);
    margo_create_wrap(ipc_update_metadentry_id, rpc_update_metadentry_id, path, handle, svr_addr, false);

    ld_logger->debug("{}() About to send RPC ...", __func__);

    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        margo_get_output(handle, &out);

        ld_logger->debug("{}() Got response success: {}", __func__, out.err);
        err = out.err;
        /* clean up resources consumed by this rpc */
        margo_free_output(handle, &out);
    } else {
        ld_logger->warn("{}() timed out", __func__);
    }

    margo_destroy(handle);
    return err;
}

int rpc_send_update_metadentry_size(const string& path, const size_t size, const off_t offset, const bool append_flag,
                                    off_t& ret_size) {
    hg_handle_t handle;
    hg_addr_t svr_addr = HG_ADDR_NULL;
    rpc_update_metadentry_size_in_t in{};
    rpc_update_metadentry_size_out_t out{};
    // add data
    in.path = path.c_str();
    in.size = size;
    in.offset = offset;
    if (append_flag)
        in.append = HG_TRUE;
    else
        in.append = HG_FALSE;
    int err = EUNKNOWN;

    ld_logger->debug("{}() Creating Mercury handle ...", __func__);
    margo_create_wrap(ipc_update_metadentry_size_id, rpc_update_metadentry_size_id, path, handle, svr_addr, false);

    ld_logger->debug("{}() About to send RPC ...", __func__);
    int send_ret = HG_FALSE;
    for (int i = 0; i < RPC_TRIES; ++i) {
        send_ret = margo_forward_timed(handle, &in, RPC_TIMEOUT);
        if (send_ret == HG_SUCCESS) {
            break;
        }
    }
    if (send_ret == HG_SUCCESS) {
        /* decode response */
        ld_logger->trace("{}() Waiting for response", __func__);
        if (margo_get_output(handle, &out) != HG_SUCCESS) {
            ld_logger->error("{}() Unable to get rpc output", __func__);
            ret_size = 0;
        } else {
            ld_logger->debug("{}() Got response success: {}", __func__, out.err);
            err = out.err;
            ret_size = out.ret_size;
            /* clean up resources consumed by this rpc */
            margo_free_output(handle, &out);
        }
    } else {
        ld_logger->warn("{}() timed out", __func__);
    }

    margo_destroy(handle);
    return err;
}