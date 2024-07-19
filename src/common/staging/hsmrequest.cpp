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
 * @brief This file prepares an HSM request
 * @internal
 * This file initialize and set up an HSM request.
 * @endinternal
 */
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/lustre/lustre_fid.h>
#include <lustre/lustreapi.h>
#include <commom/staging/hsm_requests.hpp>


namespace hsmreq {
    // Allocate memory for the hsm_user_request structure with no data (data_len = 0)
    int itemcount = 1;
    int data_len = 0;
    struct hsm_user_request *req;

    void processRequest(const Request& request) {
        struct hsm_user_request* req = llapi_hsm_user_request_alloc(itemcount, data_len);
        if (req == nullptr) {
            std::cerr << "Failed to allocate memory for hsm_user_request\n";
            return; 
        }

        // Set archive_id to 0 (assuming default archive)
        req->hur_request.hr_archive_id = 100;

        // Set flags to 0 (assuming no specific flags needed)
        req->hur_request.hr_flags = 0;

        // Set item count to 0 (no additional user items)
        req->hur_request.hr_itemcount = itemcount;

        //std::cout << "Processing request for path: " << request.path << " - Type: ";
        switch (request.type) {
            case RequestType::ARCHIVE:
                // Set the action to HUA_ARCHIVE
                req->hur_request.hr_action = HUA_ARCHIVE;
                std::cout << "Archive\n";
                break;
            case RequestType::RESTORE:
                // Set the action to HUA_RESTORE
                req->hur_request.hr_action = HUA_RESTORE;
                std::cout << "Restore\n";
                break;
        }
        
        // Send the HSM request
        int ret = llapi_hsm_request(request.path, req);
        if (ret != 0) {
            std::cerr << "Failed to archive/restore file: " << path << " (error code: " << ret << ")\n";
        } else {
            std::cout << "Successfully archived/restored file: " << path << "\n";
        }

        // Free the allocated memory
        free(req);
    }
}


