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
 * @brief This is source file for setting and destroying hsm enviroment
 * @internal
 * This file includes the setting all necessary module for HSM enviroment like copy manager
 * and copy tool and also destroying these modules.
 * @endinternal
 */


#include <thread> // Include for std::thread
#include <common/staging/hsm_extension.hpp>
#include <common/staging/stagelist.hpp>
#include "daemon/stagingdata/flushback.hpp"
#include <daemon/stagingdata/intialize_hsm.hpp>


namespace hsmenv{
    

    // Define transfer_cached_files function
    void transfer_cached_files() {
        // Call transfercachedfiles
        hsminital::transfercachedfiles();
    }
    // Define setup function

    void setup() {
        // Initialize the gekko path and lustre path for the copy tool
        hsmextension::opt.o_hsm_root = hsminital::gekkoroot;
        hsmextension::opt.opt.o_mnt = hsminital::lustreroot;
        hsmextension::opt.o_archive_cnt=1;
        hsmextension::opt.o_archive_id=100;

        // Call ct_setup to setup copy manager
        int setup_result = hsmextension::ct_setup();
        if (setup_result == 0) {
            std::cout << "ct_setup completed successfully." << std::endl;
            // Call wrapper_ct_run to run copy manager
            int rc = hsmextension::wrapper_ct_run();
        } else {
            std::cerr << "ct_setup failed with error code: " << setup_result << std::endl;
            // Handle the error or exit the program.
        }

        // Start a new thread to execute transfercachedfiles in parallel
        std::thread transfer_thread(transfer_cached_files);
        // Detach the thread to allow it to run independently
        transfer_thread.detach();
    }

     // Define shutdown function
    void shutdown() {

        //Flush remaining data to Lustre
        stageout::flushback();
        // Call wrapper_ct_shutdown to deregister copy manager from HSM Lustre
        hsmextension::wrapper_ct_shutdown();
        
    }
    

}

