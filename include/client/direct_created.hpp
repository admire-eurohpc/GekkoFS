/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  This file is part of GekkoFS.
*/
#ifndef DIRECT_CREATED_HPP
#define DIRECT_CREATED_HPP

#include <string>
#include <vector>

/**
 * @brief Add a path to the removed path set
 * 
 * @param path The path of the file.
 *
 */
void add_dc_path(const std::string& path);

/**
 * @brief Remove a path from the removed path set
 * 
 * @param path The path of the file.
 * 
 */
void remove_dc_path(const std::string& path);

/**
 * @brief Check if the path exists in the removed path set
 * 
 * @param path The path of the file.
 * @return true if the path exists, false if it doesn't.
 * 
 */
bool dc_check_path(const std::string& path);

#endif // DIRECT_CREATED_HPP
