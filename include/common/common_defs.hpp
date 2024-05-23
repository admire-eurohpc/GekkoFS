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

#ifndef GEKKOFS_COMMON_DEFS_HPP
#define GEKKOFS_COMMON_DEFS_HPP

namespace gkfs {
namespace client {
// This must be equivalent to the line set in the gkfs script
constexpr auto hostsfile_end_str = "#FS_INSTANCE_END";

} // namespace client
namespace rpc {
// These constexpr set the RPC's identity and which handler the receiver end
// should use
using chnk_id_t = unsigned long;
struct ChunkStat {
    unsigned long chunk_size;
    unsigned long chunk_total;
    unsigned long chunk_free;
};

namespace tag {

constexpr auto fs_config = "rpc_srv_fs_config";
constexpr auto create = "rpc_srv_mk_node";
constexpr auto stat = "rpc_srv_stat";
constexpr auto remove_metadata = "rpc_srv_rm_metadata";
constexpr auto remove_data = "rpc_srv_rm_data";
constexpr auto decr_size = "rpc_srv_decr_size";
constexpr auto update_metadentry = "rpc_srv_update_metadentry";
constexpr auto get_metadentry_size = "rpc_srv_get_metadentry_size";
constexpr auto update_metadentry_size = "rpc_srv_update_metadentry_size";
constexpr auto get_dirents = "rpc_srv_get_dirents";
constexpr auto get_dirents_extended = "rpc_srv_get_dirents_extended";
#ifdef HAS_SYMLINKS
constexpr auto mk_symlink = "rpc_srv_mk_symlink";
#endif
constexpr auto write = "rpc_srv_write_data";
constexpr auto read = "rpc_srv_read_data";
constexpr auto truncate = "rpc_srv_trunc_data";
constexpr auto get_chunk_stat = "rpc_srv_chunk_stat";
// IPC communication between client and proxy
constexpr auto client_proxy_create = "proxy_rpc_srv_create";
constexpr auto client_proxy_stat = "proxy_rpc_srv_stat";
constexpr auto client_proxy_remove = "proxy_rpc_srv_remove";
constexpr auto client_proxy_decr_size = "proxy_rpc_srv_decr_size";
constexpr auto client_proxy_get_size = "proxy_rpc_srv_get_metadentry_size";
constexpr auto client_proxy_update_size =
        "proxy_rpc_srv_update_metadentry_size";
constexpr auto client_proxy_write = "proxy_rpc_srv_write_data";
constexpr auto client_proxy_read = "proxy_rpc_srv_read_data";
constexpr auto client_proxy_truncate = "proxy_rpc_srv_truncate";
constexpr auto client_proxy_chunk_stat = "proxy_rpc_srv_chunk_stat";
constexpr auto client_proxy_get_dirents_extended =
        "proxy_rpc_srv_get_dirents_extended";
// Specific RPCs between daemon and proxy
constexpr auto proxy_daemon_write = "proxy_daemon_rpc_srv_write_data";
constexpr auto proxy_daemon_read = "proxy_daemon_rpc_srv_read_data";

} // namespace tag

namespace protocol {
constexpr auto na_sm = "na+sm";
constexpr auto ofi_sockets = "ofi+sockets";
constexpr auto ofi_tcp = "ofi+tcp";
constexpr auto ofi_verbs = "ofi+verbs";
constexpr auto ofi_psm2 = "ofi+psm2";
constexpr auto ucx_all = "ucx+all";
constexpr auto ucx_tcp = "ucx+tcp";
constexpr auto ucx_rc = "ucx+rc";
constexpr auto ucx_ud = "ucx+ud";
/*
 * Disable warning for unused variable
 * This is a bug in gcc that was fixed in version 13.
 * XXX Can be removed in the future
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
constexpr auto all_remote_protocols = {ofi_sockets, ofi_tcp, ofi_verbs,
                                       ofi_psm2,    ucx_all, ucx_tcp,
                                       ucx_rc,      ucx_ud};
#pragma GCC diagnostic pop
} // namespace protocol
} // namespace rpc

namespace malleable::rpc::tag {
constexpr auto expand_start = "rpc_srv_expand_start";
constexpr auto expand_status = "rpc_srv_expand_status";
constexpr auto expand_finalize = "rpc_srv_expand_finalize";
constexpr auto migrate_metadata = "rpc_srv_migrate_metadata";
constexpr auto migrate_data = "rpc_srv_migrate_data";
} // namespace malleable::rpc::tag

namespace config::syscall::stat {
// Number 512-byte blocks allocated as it is in the linux kernel (struct_stat.h)
constexpr auto st_nblocksize = 512;
} // namespace config::syscall::stat
} // namespace gkfs
#endif // GEKKOFS_COMMON_DEFS_HPP
