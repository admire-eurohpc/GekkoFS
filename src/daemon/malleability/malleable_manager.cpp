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

#include <daemon/malleability/malleable_manager.hpp>

using namespace std;

namespace gkfs::malleable {

void
MalleableManager::expand_abt(void* _arg) {

    GKFS_DATA->redist_running(true);
    GKFS_DATA->spdlogger()->info("{}() Starting metadata redistribution...",
                                 __func__);
    GKFS_DATA->malleable_manager()->redistribute_metadata();
    GKFS_DATA->spdlogger()->info("{}() Metadata redistribution completed.",
                                 __func__);
    GKFS_DATA->spdlogger()->info("{}() Starting data redistribution...",
                                 __func__);
    GKFS_DATA->malleable_manager()->redistribute_data();
    GKFS_DATA->spdlogger()->info("{}() Data redistribution completed.",
                                 __func__);
    GKFS_DATA->redist_running(false);
}

void
MalleableManager::expand_start(int old_server_conf, int new_server_conf) {
    auto abt_err =
            ABT_thread_create(RPC_DATA->io_pool(), expand_abt,
                              ABT_THREAD_ATTR_NULL, nullptr, &redist_thread_);
    if(abt_err != ABT_SUCCESS) {
        auto err_str = fmt::format(
                "MalleableManager::{}() Failed to create ABT thread with abt_err '{}'",
                __func__, abt_err);
        throw runtime_error(err_str);
    }
}
void
MalleableManager::redistribute_metadata() {
    //    reload_distribution_configuration();

    //    auto mid = RPC_DATA->client_rpc_mid();
    //    auto relocate_metadata_id =
    //            gkfs::rpc::get_rpc_id(mid, gkfs::rpc::tag::relocate_metadata);
    //
    //    auto& distributor = *(GKFS_DATA->distributor());
    //    auto hosts =
    //            dynamic_cast<gkfs::rpc::RandomSlicingDistributor*>(&distributor)
    //                    ->get_hosts_map();
    //    GKFS_DATA->spdlogger()->info("{}() Got host id = {} and parsed {}
    //    hosts",
    //                                 __func__, localhost, hosts.size());
    //
    //    // Relocate metadata
    //    for(const auto& [metakey, metavalue] : GKFS_DATA->mdb()->get_all()) {
    //        if(metakey == "/") {
    //            continue;
    //        }
    //        auto destination = distributor.locate_file_metadata(metakey);
    //
    //        GKFS_DATA->spdlogger()->trace(
    //                "{}() Metadentry {} : {} {} {}", __func__, metakey,
    //                metavalue, (destination == localhost ? " Stay on " : " ->
    //                Goto "), destination);
    //
    //        if(destination == localhost) {
    //            continue;
    //        }
    //        // send RPC
    //        rpc_relocate_metadata_in_t in{};
    //        rpc_err_out_t out{};
    //        hg_addr_t host_addr{};
    //
    //        in.key = metakey.c_str();
    //        in.value = metavalue.c_str();
    //
    //        auto ret = margo_addr_lookup(mid,
    //        hosts[destination].second.c_str(),
    //                                     &host_addr);
    //        assert(ret == HG_SUCCESS);
    //
    //        // let's do this sequential first
    //        hg_handle_t handle;
    //        ret = margo_create(mid, host_addr, relocate_metadata_id, &handle);
    //        assert(ret == HG_SUCCESS);
    //
    //        ret = margo_forward(handle, &in); // blocking
    //        assert(ret == HG_SUCCESS);
    //
    //        ret = margo_get_output(handle, &out);
    //        assert(ret == HG_SUCCESS);
    //
    //        // TODO(dauer) catch DB exceptions
    //        GKFS_DATA->mdb()->remove(in.key);
    //
    //        if(HG_SUCCESS !=
    //           gkfs::rpc::margo_client_cleanup(&handle, &out, &mid,
    //           &host_addr)) {
    //            GKFS_DATA->spdlogger()->error("{}() Error during margo
    //            cleanup",
    //                                          __func__);
    //        }
    //    }
}

void
MalleableManager::redistribute_data() {
    // Relocate data (chunks)
    //    auto relocate_chunk_rpc_id =
    //            gkfs::rpc::get_rpc_id(mid, gkfs::rpc::tag::relocate_chunk);
    //    for(auto& chunks_dir :
    //    GKFS_DATA->storage()->chunks_directory_iterator()) {
    //        if(!chunks_dir.is_directory()) {
    //            GKFS_DATA->spdlogger()->warn(
    //                    "{}() Expected directory but got something else: {}",
    //                    __func__, chunks_dir.path().string());
    //            continue;
    //        }
    //        string file_path = GKFS_DATA->storage()->get_file_path(
    //                chunks_dir.path().filename().string());
    //
    //        for(auto& chunk_file : fs::directory_iterator(chunks_dir)) {
    //            if(!chunk_file.is_regular_file()) {
    //                GKFS_DATA->spdlogger()->warn(
    //                        "{}() Expected regular file but got something
    //                        else: {}",
    //                        __func__, chunk_file.path().string());
    //                continue;
    //            }
    //            gkfs::rpc::chnk_id_t chunk_id =
    //                    std::stoul(chunk_file.path().filename().string());
    //            auto destination = distributor.locate_data(file_path,
    //            chunk_id); size_t size = chunk_file.file_size();
    //
    //            GKFS_DATA->spdlogger()->trace(
    //                    "{}() Checking {} chunk: {} size: {} {} {}", __func__,
    //                    file_path, chunk_id, size,
    //                    (destination == localhost ? " Stay on" : " -> Goto "),
    //                    destination);
    //
    //            if(destination == localhost) {
    //                continue;
    //            }
    //
    //            // prepare bulk
    //            unique_ptr<char[]> buf(new char[size]());
    //            // read data (blocking)
    //            hg_size_t bytes_read = GKFS_DATA->storage()->read_chunk(
    //                    file_path, chunk_id, buf.get(), size, 0);
    //            hg_bulk_t bulk{};
    //            char* bufptr = buf.get();
    //            auto ret = margo_bulk_create(mid, 1, (void**) &bufptr,
    //            &bytes_read,
    //                                         HG_BULK_READ_ONLY, &bulk);
    //            assert(ret == HG_SUCCESS);
    //
    //            // send RPC
    //            rpc_relocate_chunk_in_t in{};
    //            rpc_err_out_t out{};
    //            hg_addr_t host_addr{};
    //
    //            in.path = file_path.c_str();
    //            in.chunk_id = chunk_id;
    //            in.bulk_handle = bulk;
    //
    //            ret = margo_addr_lookup(mid,
    //            hosts[destination].second.c_str(),
    //                                    &host_addr);
    //            assert(ret == HG_SUCCESS);
    //
    //            // let's do this sequential first
    //            hg_handle_t handle;
    //            ret = margo_create(mid, host_addr, relocate_chunk_rpc_id,
    //            &handle); assert(ret == HG_SUCCESS);
    //
    //            ret = margo_forward(handle, &in); // blocking
    //            assert(ret == HG_SUCCESS);
    //
    //            ret = margo_get_output(handle, &out);
    //            assert(ret == HG_SUCCESS);
    //
    //            // TODO(dauer) process output
    //            GKFS_DATA->storage()->remove_chunk(file_path, chunk_id);
    //
    //            // FIXME This can leave behind empty directories, even when
    //            the
    //            // whole file is delete later. Three possibilities:
    //            // 1) Clean them up, but make sure this doesn't break another
    //            thread
    //            // creating a new chunk in this directory at the same time.
    //            // 2) Switch to a flat namespace without directories
    //            // 3) Ignore and waste some inodes
    //
    //            if(HG_SUCCESS != gkfs::rpc::margo_client_cleanup(
    //                                     &handle, &out, &mid, &host_addr,
    //                                     &bulk)) {
    //                cout << "Error during margo cleanup.\n";
    //            }
    //        }
    //    }
}

} // namespace gkfs::malleable
