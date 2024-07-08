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
#include <client/rpc/forward_malleability.hpp>
#include <client/preload.hpp>
#include <client/preload_util.hpp>
#include <client/logging.hpp>
#include <client/rpc/rpc_types.hpp>
#include <common/rpc/distributor.hpp>

namespace gkfs::malleable::rpc {

int
forward_expand_start(int old_server_conf, int new_server_conf) {
    LOG(INFO, "{}() enter", __func__);
    auto const targets = CTX->distributor()->locate_directory_metadata();

    auto err = 0;
    // send async RPCs
    std::vector<hermes::rpc_handle<gkfs::malleable::rpc::expand_start>> handles;

    for(std::size_t i = 0; i < targets.size(); ++i) {

        // Setup rpc input parameters for each host
        auto endp = CTX->hosts().at(targets[i]);

        gkfs::malleable::rpc::expand_start::input in(old_server_conf,
                                                     new_server_conf);

        try {
            LOG(DEBUG, "{}() Sending RPC to host: '{}'", __func__, targets[i]);
            handles.emplace_back(
                    ld_network_service
                            ->post<gkfs::malleable::rpc::expand_start>(endp,
                                                                       in));
        } catch(const std::exception& ex) {
            LOG(ERROR,
                "{}() Unable to send non-blocking forward_expand_start() [peer: {}] err '{}'",
                __func__, targets[i], ex.what());
            err = EBUSY;
            break; // we need to gather responses from already sent RPCS
        }
    }

    LOG(INFO, "{}() send expand_start rpc to '{}' targets", __func__,
        targets.size());

    // wait for RPC responses
    for(std::size_t i = 0; i < handles.size(); ++i) {

        gkfs::malleable::rpc::expand_start::output out;

        try {
            out = handles[i].get().at(0);

            if(out.err() != 0) {
                LOG(ERROR,
                    "{}() Failed to retrieve dir entries from host '{}'. Error '{}'",
                    __func__, targets[i], strerror(out.err()));
                err = out.err();
                // We need to gather all responses before exiting
                continue;
            }
        } catch(const std::exception& ex) {
            LOG(ERROR,
                "{}() Failed to get rpc output.. [target host: {}] err '{}'",
                __func__, targets[i], ex.what());
            err = EBUSY;
            // We need to gather all responses before exiting
            continue;
        }
    }
    return err;
}

int
forward_expand_status() {
    LOG(INFO, "{}() enter", __func__);
    auto const targets = CTX->distributor()->locate_directory_metadata();

    auto err = 0;
    // send async RPCs
    std::vector<hermes::rpc_handle<gkfs::malleable::rpc::expand_status>>
            handles;

    for(std::size_t i = 0; i < targets.size(); ++i) {

        // Setup rpc input parameters for each host
        auto endp = CTX->hosts().at(targets[i]);

        try {
            LOG(DEBUG, "{}() Sending RPC to host: '{}'", __func__, targets[i]);
            handles.emplace_back(
                    ld_network_service
                            ->post<gkfs::malleable::rpc::expand_status>(endp));
        } catch(const std::exception& ex) {
            LOG(ERROR,
                "{}() Unable to send non-blocking forward_expand_status() [peer: {}] err '{}'",
                __func__, targets[i], ex.what());
            err = EBUSY;
            break; // we need to gather responses from already sent RPCS
        }
    }

    LOG(INFO, "{}() send expand_status rpc to '{}' targets", __func__,
        targets.size());

    // wait for RPC responses
    for(std::size_t i = 0; i < handles.size(); ++i) {
        gkfs::malleable::rpc::expand_status::output out;
        try {
            out = handles[i].get().at(0);
            if(out.err() > 0) {
                LOG(DEBUG,
                    "{}() Host '{}' not done yet with malleable operation.",
                    __func__, targets[i]);
                err += out.err();
            }
            if(out.err() < 0) {
                // ignore. shouldn't happen for now
                LOG(ERROR,
                    "{}() Host '{}' is unable to check for expansion progress. (shouldn't happen)",
                    __func__, targets[i]);
            }
        } catch(const std::exception& ex) {
            LOG(ERROR,
                "{}() Failed to get rpc output.. [target host: {}] err '{}'",
                __func__, targets[i], ex.what());
            err = EBUSY;
            // We need to gather all responses before exiting
            continue;
        }
    }
    return err;
}

int
forward_expand_finalize() {
    LOG(INFO, "{}() enter", __func__);
    auto const targets = CTX->distributor()->locate_directory_metadata();

    auto err = 0;
    // send async RPCs
    std::vector<hermes::rpc_handle<gkfs::malleable::rpc::expand_finalize>>
            handles;

    for(std::size_t i = 0; i < targets.size(); ++i) {

        // Setup rpc input parameters for each host
        auto endp = CTX->hosts().at(targets[i]);

        try {
            LOG(DEBUG, "{}() Sending RPC to host: '{}'", __func__, targets[i]);
            handles.emplace_back(
                    ld_network_service
                            ->post<gkfs::malleable::rpc::expand_finalize>(
                                    endp));
        } catch(const std::exception& ex) {
            LOG(ERROR,
                "{}() Unable to send non-blocking forward_expand_finalize() [peer: {}] err '{}'",
                __func__, targets[i], ex.what());
            err = EBUSY;
            break; // we need to gather responses from already sent RPCS
        }
    }

    LOG(INFO, "{}() send expand_finalize rpc to '{}' targets", __func__,
        targets.size());

    // wait for RPC responses
    for(std::size_t i = 0; i < handles.size(); ++i) {

        gkfs::malleable::rpc::expand_finalize::output out;

        try {
            out = handles[i].get().at(0);

            if(out.err() != 0) {
                LOG(ERROR, "{}() Failed finalize on host '{}'. Error '{}'",
                    __func__, targets[i], strerror(out.err()));
                err = out.err();
                // We need to gather all responses before exiting
                continue;
            }
        } catch(const std::exception& ex) {
            LOG(ERROR,
                "{}() Failed to get rpc output.. [target host: {}] err '{}'",
                __func__, targets[i], ex.what());
            err = EBUSY;
            // We need to gather all responses before exiting
            continue;
        }
    }
    return err;
}

} // namespace gkfs::malleable::rpc