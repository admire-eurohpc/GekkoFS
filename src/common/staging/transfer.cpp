
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
 * @brief The main source file to manage data transfer between Gekkofs and Lustre.
 * @internal
 * This file includes all operations related to triggering cargo transfer operations
 * @endinternal
 */

#include <cargo.hpp>
#include <common/staging/transfer.hpp>
#include <iostream>
#include <lustre/lustreapi.h>
#include <lustre/lustre_user.h>
#include <fstream>
#include <cerrno>
#include <common/staging/transfer.hpp>
#include <common/staging/stagelist.hpp>



namespace transfer {
    using cargo::server;
    using cargo::dataset;
    using cargo::transfer;
    int count_lock=1;
    int root_lock=1;
    std::string srcroot,dstroot;

    // Initialize the Cargo server
    std::string get_initialized_server() {
        //cargo::server srv;
        std::string address;
        // Read server address from file
        if (count_lock==1)
        {
            std::ifstream file("etc/config.conf");
            if (file.is_open()) {
                
                std::getline(file, address);
                cargo::server srv(address);
                file.close();
            } else {
                std::cerr << "Error opening server address file\n";
            }
            count_lock=0;
        }
        else
        {
            return address;
        }
        return address;

        //return srv;
    }

    /// @brief  Transfer a file from lustre to gekko
    /// @param lustrepath address of file in lustre
    /// @param gekkopath address of file in Gekkofs
    /// @return success or failure
    bool transferfilegekko(const std::string& lustrepath, const std::string& gekkopath) {
        //Move this part
        // Create a Cargo server with the appropriate address
        cargo::server srv(get_initialized_server());
        struct stat st_temp;
        int syserror =1 ;


        // Create the source and target datasets 
        std::vector<dataset> sources, targets;
        sources.emplace_back(lustrepath, dataset::type::posix);
        targets.emplace_back(gekkopath, dataset::type::posix);

        // Transfer the datasets

        const auto transfer_result = cargo::transfer_datasets(srv, source, target);
        // wait for transfer
        auto s = transfer_result.wait();

        // Transfer was successful
        if (s.state()== cargo::transfer_state::completed && s.error() == cargo::error_code::success) {

         return true;
        }
        else
        {   
        std::cerr << "Error transferring datasets: " << s.error() << '\n';
            return false;
        }   

    

    }
    /// @brief  Transfer files from lustre to gekko
    /// @param lustrepath address of files in lustre
    /// @param gekkopath address of files in Gekkofs
    /// @return success or failure

    bool transferdirgekko(const std::vector<std::string>& lustrepaths, const std::vector<std::string>& gekkopaths) {
        // Create a Cargo server with the appropriate address
        cargo::server srv(get_initialized_server());
        struct stat st_temp;
        int syserror =1 ;

        // Create the source and target datasets
        std::vector<dataset> sources, targets;
        for (const auto& lustrepath : lustrepaths) {
                sources.emplace_back(lustrepath, dataset::type::posix)
                

        }
        for (const auto& gekkopath : gekkopaths) {
            targets.emplace_back(gekkopath, dataset::type::posix);}

	// Transfer the datasets

        const auto transfer_result = cargo::transfer_datasets(srv, source, target);
        // wait for transfer
        auto s = transfer_result.wait();

        // Transfer was successful
        if (s.state()== cargo::transfer_state::completed && s.error() == cargo::error_code::success) {

         return true;
        }
        else
        {   
        std::cerr << "Error transferring datasets: " << s.error() << '\n';
            return false;
        }
    }
    /// @brief  Transfer files from lustre to gekko
    /// @param lustrepath address of files in lustre
    /// @param gekkopath address of files in Gekkofs
    /// @return success or failure       

    bool transferdirlustre(const std::vector<std::string>& gekkopaths, const std::vector<std::string>& lustrepaths) {
        // Create a Cargo server with the appropriate address
        cargo::server srv(get_initialized_server());
        struct stat st_temp;
        int syserror =1 ;
        // Create the source and target datasets
        std::vector<dataset> sources, targets;
       for (size_t i = 0; i < gekkopaths.size(); ++i) {
            // Stop any other access
            add_rm_path(gekkopaths[i]);
            sources.emplace_back(gekkopaths[i], dataset::type::posix);
            targets.emplace_back(lustrepaths[i], dataset::type::parallel);
            // Transfer the datasets
        const auto transfer_result = cargo::transfer_datasets(srv, source, target);
        // wait for transfer
        auto s = transfer_result.wait();

        // Transfer was successful
        if (s.state()== cargo::transfer_state::completed && s.error() == cargo::error_code::success) {
            return true;

        }

        else
        {
            std::cerr << "Error transferring datasets: " << s.error() << '\n';
            return false;
        }
    }
    /// @brief  Transfer files from gekko to Lustre
    /// @param lustrepath address of files in lustre
    /// @param gekkopath address of files in Gekkofs
    /// @return success or failure 
    bool transferfilelustre(const std::string& gekkopath, const std::string& lustrepath) {
        // Create a Cargo server with the appropriate address
        cargo::server srv(get_initialized_server());
        struct stat st_temp;
        int syserror =1 ;
        // Create the source and target datasets
        std::vector<dataset> source, target;
        source.emplace_back(gekkopath, dataset::type::posix);
        target.emplace_back(lustrepath, dataset::type::parallel);
        add_rm_path(path);
        // Transfer the datasets
        const auto transfer_result = cargo::transfer_datasets(srv, source, target);
        // wait for transfer
        auto s = transfer_result.wait();

        // Transfer was successful
        if (s.state()== cargo::transfer_state::completed && s.error() == cargo::error_code::success) {
            return false;
        }
        else
        {
            std::cerr << "Error transferring datasets: " << s.error() << '\n';
            return true;
        }
        
        
        
    }
    void intializeroot()
    {
        if (root_lock==1)
        {
            srcroot = hsminital::lustreroot;
            dstroot = hsminital::gekkoroot;
            root_lock=0;
        }
        return();

    }
    bool transferlustre (const std::string& path)
    {
        std::string transferpath =path;

        if (transferpath.find(dstroot) == 0) {
                    // Replace lustreroot with gekkoroot
                    std::string replaced_transfer = srcroot + line.substr(dstroot.size());
                    // Call transfer::transfer_posix with the modified line
                    bool result = transfer::transferfilelustre(transferpath, replaced_transfer);
                    if (!result) {
                        std::cerr << "Error: Transfer of file from " << line << " to " << replaced_line << " failed." << std::endl;
                        return false; // Exit with error
                    }
                    return true;
                }
                return false;
    }
    bool transfergekko (const std::string& path)
    {
        std::string transferpath =path;

        if (transferpath.find(srcroot) == 0) {
                    // Replace lustreroot with gekkoroot
                    std::string replaced_transfer = dstroot + line.substr(srcroot.size());
                    // Call transfer::transfer_posix with the modified line
                    bool result = transfer::transferfilegekko(transferpath, replaced_transfer);
                    if (!result) {
                        std::cerr << "Error: Transfer of file from " << line << " to " << replaced_line << " failed." << std::endl;
                        return false; // Exit with error
                    }
                    return true;
                }
                return false;

    }
}






