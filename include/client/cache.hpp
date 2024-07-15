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

#ifndef GKFS_CLIENT_CACHE
#define GKFS_CLIENT_CACHE

#include <client/open_file_map.hpp>

#include <ctime>
#include <functional>
#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <cstdint>
#include <utility>

namespace gkfs::cache {

namespace dir {

/**
 * @brief Cache entry metadata.
 * The entries are limited to the get_dir_extended RPC.
 */
struct cache_entry {
    gkfs::filemap::FileType file_type;
    uint64_t size;
    time_t ctime;
};

/**
 * @brief Cache for directory entries to accelerate ls -l type operations
 */
class DentryCache {
private:
    // <dir_id, <name, cache_entry>>: Associate a directory id with its entries
    // containing the directory name and cache entry metadata
    std::unordered_map<uint32_t, std::unordered_map<std::string, cache_entry>>
            entries_;
    // <dir_path, dir_id>: Associate a directory path with a unique id
    std::unordered_map<std::string, uint32_t> entry_dir_id_;
    std::mutex mtx_;                 // Mutex to protect the cache
    std::hash<std::string> str_hash; // hash to generate ids

    /**
     * @brief Generate a unique id for caching a directory
     * @param dir_path
     * @return id
     */
    uint32_t
    gen_dir_id(const std::string& dir_path);

    /**
     * @brief Get the unique id for a directory to retrieve its entries. Creates
     * an id if it does not exist.
     * @param dir_path
     * @return id
     */
    uint32_t
    get_dir_id(const std::string& dir_path);

public:
    DentryCache() = default;

    virtual ~DentryCache() = default;

    /**
     * @brief Insert a new entry in the cache
     * @param parent_dir
     * @param name
     * @param value
     */
    void
    insert(const std::string& parent_dir, std::string name, cache_entry value);

    /**
     * @brief Get an entry from the cache for a given directory
     * @param parent_dir
     * @param name
     * @return std::optional<cache_entry>
     */
    std::optional<cache_entry>
    get(const std::string& parent_dir, const std::string& name);

    /**
     * @brief Clear the cache for a given directory. Called when a directory is
     * closed
     * @param dir_path
     */
    void
    clear_dir(const std::string& dir_path);

    /**
     * @brief Dump the cache to the log for debugging purposes. Not used in
     * production.
     * @param dir_path
     */
    void
    dump_cache_to_log(const std::string& dir_path);

    /**
     * @brief Clear the entire cache
     */
    void
    clear();
};
} // namespace dir

namespace file {
class WriteSizeCache {
private:
    // <path<cnt, size>>
    std::unordered_map<std::string, std::pair<size_t, size_t>> size_cache;
    std::mutex mtx_;

    // Flush threshold in number of write ops per file
    size_t flush_threshold_{0};

public:
    WriteSizeCache() = default;

    virtual ~WriteSizeCache() = default;

    /**
     * @brief Record the size of a file and add it to the cache
     * @param path gekkofs path
     * @param size current size to set for given path
     * @return [size_update counter, current cached size]
     */
    std::pair<size_t, size_t>
    record(std::string path, size_t size);

    /**
     * @brief reset entry from the cache
     * @param path
     * @param evict if true, entry is removed from cache, reseted to cnt 0
     * otherwise
     * @return [size_update counter, current cached size]
     */
    std::pair<size_t, size_t>
    reset(const std::string& path, bool evict);

    /**
     * @brief Flush the cache for a given path contacting the corresponding
     * daemon
     * @param path
     * @param evict during flush: if true, entry is removed from cache, reseted
     * to cnt 0 otherwise
     * @return error code and flushed size
     */
    std::pair<int, off64_t>
    flush(const std::string& path, bool evict = true);


    // GETTER/SETTER
    size_t
    flush_threshold() const;

    void
    flush_threshold(size_t flush_threshold);
};
} // namespace file
} // namespace gkfs::cache

#endif // GKFS_CLIENT_CACHE
