/*
  Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/

#ifndef GEKKOFS_PROXY_RPC_UTIL_HPP
#define GEKKOFS_PROXY_RPC_UTIL_HPP

extern "C" {
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <margo.h>
}

#include <string>

namespace gkfs {
namespace rpc {

// TODO THIS WHOLE THING IS COPY PASTA FROM DAEMON. REFACTOR. It is literally
// not allowed to merge with this todo!
// XXX Also file name = stupid.

template <typename InputType, typename OutputType>
inline hg_return_t
cleanup(hg_handle_t* handle, InputType* input, OutputType* output,
        hg_bulk_t* bulk_handle) {
    auto ret = HG_SUCCESS;
    if(bulk_handle) {
        ret = margo_bulk_free(*bulk_handle);
        if(ret != HG_SUCCESS)
            return ret;
    }
    if(input && handle) {
        ret = margo_free_input(*handle, input);
        if(ret != HG_SUCCESS)
            return ret;
    }
    if(output && handle) {
        ret = margo_free_output(*handle, output);
        if(ret != HG_SUCCESS)
            return ret;
    }
    if(handle) {
        ret = margo_destroy(*handle);
        if(ret != HG_SUCCESS)
            return ret;
    }
    return ret;
}

template <typename OutputType>
inline hg_return_t
respond(hg_handle_t* handle, OutputType* output) {
    auto ret = HG_SUCCESS;
    if(output && handle) {
        ret = margo_respond(*handle, output);
        if(ret != HG_SUCCESS)
            return ret;
    }
    return ret;
}

template <typename InputType, typename OutputType>
inline hg_return_t
cleanup_respond(hg_handle_t* handle, InputType* input, OutputType* output,
                hg_bulk_t* bulk_handle) {
    auto ret = respond(handle, output);
    if(ret != HG_SUCCESS)
        return ret;
    return cleanup(handle, input, static_cast<OutputType*>(nullptr),
                   bulk_handle);
}

template <typename InputType, typename OutputType>
inline hg_return_t
cleanup_respond(hg_handle_t* handle, InputType* input, OutputType* output) {
    return cleanup_respond(handle, input, output, nullptr);
}

template <typename OutputType>
inline hg_return_t
cleanup_respond(hg_handle_t* handle, OutputType* output) {
    auto ret = respond(handle, output);
    if(ret != HG_SUCCESS)
        return ret;
    if(handle) {
        ret = margo_destroy(*handle);
        if(ret != HG_SUCCESS)
            return ret;
    }
    return ret;
}

} // namespace rpc
} // namespace gkfs


#endif // GEKKOFS_PROXY_RPC_UTIL_HPP
