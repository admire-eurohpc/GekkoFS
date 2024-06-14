/*
  Copyright 2018-2020, Barcelona Supercomputing Center (BSC), Spain
  Copyright 2015-2020, Johannes Gutenberg Universitaet Mainz, Germany

  This software was partially supported by the
  EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).

  This software was partially supported by the
  ADA-FS project under the SPPEXA project funded by the DFG.

  SPDX-License-Identifier: MIT
*/
#ifndef GEKKOFS_PROXY_PROXY_HPP
#define GEKKOFS_PROXY_PROXY_HPP

// std libs
#include <string>
#include <spdlog/spdlog.h>

#include <config.hpp>
#include <common/common_defs.hpp>

// margo
extern "C" {
#include <abt.h>
#include <mercury.h>
#include <margo.h>
}

#include <proxy/proxy_data.hpp>

#define PROXY_DATA                                                             \
    (static_cast<gkfs::proxy::ProxyData*>(                                     \
            gkfs::proxy::ProxyData::getInstance()))

#endif // GEKKOFS_PROXY_PROXY_HPP
