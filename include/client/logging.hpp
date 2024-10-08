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

#ifndef LIBGKFS_LOGGING_HPP
#define LIBGKFS_LOGGING_HPP

#ifndef BYPASS_SYSCALL
#include <libsyscall_intercept_hook_point.h>
#else
#include <client/void_syscall_intercept.hpp>
#endif

#include <type_traits>
#include <client/make_array.hpp>
#include <client/syscalls.hpp>
#include <optional>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <hermes.hpp>

#ifdef GKFS_DEBUG_BUILD
#include <bitset>
#endif

namespace gkfs::log {

enum class log_level : unsigned int {
    print_syscalls = 1 << 0,
    print_syscalls_entry = 1 << 1,
    print_info = 1 << 2,
    print_critical = 1 << 3,
    print_errors = 1 << 4,
    print_warnings = 1 << 5,
    print_hermes = 1 << 6,
    print_mercury = 1 << 7,
    print_debug = 1 << 8,
    print_trace_reads = 1 << 9,

    // for internal use
    print_none = 0,
    print_all = print_syscalls | print_syscalls_entry | print_info |
                print_critical | print_errors | print_warnings | print_hermes |
                print_mercury | print_debug,
    print_most = print_all & ~print_syscalls_entry,
    print_help = 1 << 10
};

inline constexpr log_level
operator&(log_level l1, log_level l2) {
    return log_level(static_cast<short>(l1) & static_cast<short>(l2));
}

inline constexpr log_level
operator|(log_level l1, log_level l2) {
    return log_level(static_cast<short>(l1) | static_cast<short>(l2));
}

inline constexpr log_level
operator^(log_level l1, log_level l2) {
    return log_level(static_cast<short>(l1) ^ static_cast<short>(l2));
}

inline constexpr log_level
operator~(log_level l1) {
    return log_level(~static_cast<short>(l1));
}

inline constexpr bool
operator!(log_level dm) {
    return static_cast<short>(dm) == 0;
}

inline const log_level&
operator|=(log_level& l1, log_level l2) {
    return l1 = l1 | l2;
}

inline const log_level&
operator&=(log_level& l1, log_level l2) {
    return l1 = l1 & l2;
}

inline const log_level&
operator^=(log_level& l1, log_level l2) {
    return l1 = l1 ^ l2;
}


static const auto constexpr syscall = log_level::print_syscalls;
static const auto constexpr syscall_at_entry = log_level::print_syscalls_entry;
static const auto constexpr info = log_level::print_info;
static const auto constexpr critical = log_level::print_critical;
static const auto constexpr error = log_level::print_errors;
static const auto constexpr warning = log_level::print_warnings;
static const auto constexpr hermes = log_level::print_hermes;
static const auto constexpr mercury = log_level::print_mercury;
static const auto constexpr debug = log_level::print_debug;
static const auto constexpr trace_reads = log_level::print_trace_reads;
static const auto constexpr none = log_level::print_none;
static const auto constexpr most = log_level::print_most;
static const auto constexpr all = log_level::print_all;
static const auto constexpr help = log_level::print_help;

static const auto constexpr level_names = utils::make_array(
        "syscall",
        "syscall", // sycall_entry uses the same name as syscall
        "info", "critical", "error", "warning", "hermes", "mercury", "debug",
        "trace_reads");

inline constexpr auto
lookup_level_name(log_level l) {

    assert(l != log::none && l != log::help);

    // since all log levels are powers of 2, we can find a name
    // very efficiently by counting the number of trailing 0-bits in l
    const auto i = __builtin_ctz(static_cast<short>(l));
    assert(i >= 0 && static_cast<std::size_t>(i) < level_names.size());

    return level_names.at(i);
}


// forward declaration
struct logger;

namespace detail {

template <typename Buffer>
static inline void
log_buffer(std::FILE* fp, Buffer&& buffer) {
    log_buffer(::fileno(fp), std::forward<Buffer>(buffer));
}

template <typename Buffer>
static inline void
log_buffer(int fd, Buffer&& buffer) {

    if(fd < 0) {
        throw std::runtime_error("Invalid file descriptor");
    }

    ::syscall_no_intercept(SYS_write, fd, buffer.data(), buffer.size());
}

static inline void
log_buffer(int fd, const void* buffer, std::size_t length) {
    if(fd < 0) {
        throw std::runtime_error("Invalid file descriptor");
    }

    ::syscall_no_intercept(SYS_write, fd, buffer, length);
}

/**
 * @brief convert a time_t to a tm
 * It is not POSIX compliant, but it dows not uses any syscall or timezone
 * Converts a Unix timestamp (number of seconds since the beginning of 1970
 * CE) to a Gregorian civil date-time tuple in GMT (UTC) time zone.
 *
 * This conforms to C89 (and C99...) and POSIX.
 *
 * This implementation works, and doesn't overflow for any sizeof(time_t).
 * It doesn't check for overflow/underflow in tm->tm_year output. Other than
 * that, it never overflows or underflows. It assumes that that time_t is
 * signed.
 *
 * This implements the inverse of the POSIX formula
 * (http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_15)
 * for all time_t values, no matter the size, as long as tm->tm_year doesn't
 * overflow or underflow. The formula is: tm_sec + tm_min*60 + tm_hour*3600
 * + tm_yday*86400 + (tm_year-70)*31536000 + ((tm_year-69)/4)*86400 -
 * ((tm_year-1)/100)*86400 + ((tm_year+299)/400)*86400.
 *
 * License : GNU General Public License v2.0 from
 * https://github.com/pts/minilibc686/
 * @param time_t
 * @return tm
 */

static inline struct tm*
mini_gmtime_r(const time_t* timep, struct tm* tm) {
    const time_t ts = *timep;
    time_t t = ts / 86400;
    unsigned hms =
            ts %
            86400; /* -86399 <= hms <= 86399. This needs sizeof(int) >= 4. */
    time_t c, f;
    unsigned yday; /* 0 <= yday <= 426. Also fits to an `unsigned short', but
                      `int' is faster. */
    unsigned a; /* 0 <= a <= 2133. Also fits to an `unsigned short', but `int'
                   is faster. */
    if((int) hms < 0) {
        --t;
        hms += 86400;
    } /* Fix quotient and negative remainder if ts was negative (i.e. before
         year 1970 CE). */
    /* Now: -24856 <= t <= 24855. */
    tm->tm_sec = hms % 60;
    hms /= 60;
    tm->tm_min = hms % 60;
    tm->tm_hour = hms / 60;
    if(sizeof(time_t) >
       4) { /* Optimization. For int32_t, this would keep t intact, so we won't
               have to do it. This produces unreachable code. */
        f = (t + 4) % 7;
        if(f < 0)
            f += 7; /* Fix negative remainder if (t + 4) was negative. */
        /* Now 0 <= f <= 6. */
        tm->tm_wday = f;
        c = (t << 2) + 102032;
        f = c / 146097;
        if(c % 146097 < 0)
            --f; /* Fix negative remainder if c was negative. */
        --f;
        t += f;
        f >>= 2;
        t -= f;
        f = (t << 2) + 102035;
        c = f / 1461;
        if(f % 1461 < 0)
            --c; /* Fix negative remainder if f was negative. */
    } else {
        tm->tm_wday = (t + 24861) % 7; /* t + 24861 >= 0. */
        /* Now: -24856 <= t <= 24855. */
        c = ((t << 2) + 102035) / 1461;
    }
    yday = t - 365 * c - (c >> 2) + 25568;
    /* Now: 0 <= yday <= 425. */
    a = yday * 5 + 8;
    /* Now: 8 <= a <= 2133. */
    tm->tm_mon = a / 153;
    a %= 153; /* No need to fix if a < 0, because a cannot be negative here. */
    /* Now: 2 <= tm->tm_mon <= 13. */
    /* Now: 0 <= a <= 152. */
    tm->tm_mday = 1 + a / 5; /* No need to fix if a < 0, because a cannot be
                                negative here. */
    /* Now: 1 <= tm->tm_mday <= 31. */
    if(tm->tm_mon >= 12) {
        tm->tm_mon -= 12;
        /* Now: 0 <= tm->tm_mon <= 1. */
        ++c;
        yday -= 366;
    } else { /* Check for leap year (in c). */
        /* Now: 2 <= tm->tm_mon <= 11. */
        /* 1903: not leap; 1904: leap, 1900: not leap; 2000: leap */
        /* With sizeof(time_t) == 4, we have 1901 <= year <= 2038; of these
         * years only 2000 is divisble by 100, and that's a leap year, no we
         * optimize the check to `(c & 3) == 0' only.
         */
        if(!((c & 3) == 0 &&
             (sizeof(time_t) <= 4 || c % 100 != 0 || (c + 300) % 400 == 0)))
            --yday; /* These `== 0' comparisons work even if c < 0. */
    }
    tm->tm_year =
            c; /* This assignment may overflow or underflow, we don't check it.
                  Example: time_t is a huge int64_t, tm->tm_year is int32_t. */
    /* Now: 0 <= tm->tm_mon <= 11. */
    /* Now: 0 <= yday <= 365. */
    tm->tm_yday = yday;
    tm->tm_isdst = 0;
    return tm;
}

static inline struct tm*
mini_gmtime(const time_t* timep) {
    static struct tm tm;
    return mini_gmtime_r(timep, &tm);
}

static inline ssize_t
format_timeval(struct timeval* tv, char* buf, size_t sz) {
    ssize_t written = -1;
    struct tm* gm = mini_gmtime(&tv->tv_sec);


    written = (ssize_t) strftime(buf, sz, "%Y-%m-%d %H:%M:%S", gm);
    if((written > 0) && ((size_t) written < sz)) {
        int w = snprintf(buf + written, sz - (size_t) written, ".%06ld",
                         tv->tv_usec);
        written = (w > 0) ? written + w : -1;
    }

    return written;
}

/**
 * format_timestamp_to - safely format a timestamp for logging messages
 *
 * This function produes a timestamp that can be used to prefix logging
 * messages. Since we are actively intercepting system calls, the formatting
 * MUST NOT rely on internal system calls, otherwise we risk recursively
 * calling ourselves for each syscall generated. Also, we cannot rely on
 * the C formatting functions asctime, ctime, gmtime, localtime, mktime,
 * asctime_r, ctime_r, gmtime_r, localtime_r, since they acquire a
 * non-reentrant lock to determine the caller's timezone (yes, the assumedly
 * reentrant *_r versions of the functions exhibit this problem as well,
 * see https://sourceware.org/bugzilla/show_bug.cgi?id=16145). To solve this
 * issue and still get readable timestamps, we determine and cache the
 * timezone when the logger is created so that the lock is only held once, by
 * one thread exactly, and we pass it as an argument whenever we need to
 * format a timestamp. If no timezone is provided, we just format the epoch.
 *
 */
template <typename Buffer>
static inline void
format_timestamp_to(Buffer&& buffer) {

    struct ::timeval tv;

    int rv = ::syscall_no_intercept(SYS_gettimeofday, &tv, NULL);

    if(::syscall_error_code(rv) != 0) {
        return;
    }

    char buf[28];

    if(format_timeval(&tv, buf, sizeof(buf)) > 0) {
        fmt::format_to(std::back_inserter(buffer), "[{}] ", buf);
    }
}

template <typename Buffer>
static inline void
format_syscall_info_to(Buffer&& buffer, gkfs::syscall::info info) {

    const auto ttid = syscall_no_intercept(SYS_gettid);
    fmt::format_to(std::back_inserter(buffer), "[{}] [syscall] ", ttid);

    char o;
    char t;

    switch(gkfs::syscall::origin(info)) {
        case gkfs::syscall::from_internal_code:
            o = 'i';
            break;
        case gkfs::syscall::from_external_code:
            o = 'a';
            break;
        default:
            o = '?';
            break;
    }

    switch(gkfs::syscall::target(info)) {
        case gkfs::syscall::to_hook:
            t = 'h';
            break;
        case gkfs::syscall::to_kernel:
            t = 'k';
            break;
        default:
            t = '?';
            break;
    }

    const std::array<char, 5> tmp = {'[', o, t, ']', ' '};
    fmt::format_to(std::back_inserter(buffer),
                   fmt::string_view(tmp.data(), tmp.size()));
}

} // namespace detail

enum { max_buffer_size = LIBGKFS_LOG_MESSAGE_SIZE };

using static_buffer = fmt::basic_memory_buffer<char, max_buffer_size>;

struct logger {

    logger(const std::string& opts, const std::string& path,
           bool log_per_process, bool trunc
#ifdef GKFS_DEBUG_BUILD
           ,
           const std::string& filter, int verbosity
#endif
    );

    ~logger();

    template <typename... Args>
    inline void
    log(log_level level, const char* const func, const int lineno,
        Args&&... args) {

        if(!(level & log_mask_)) {
            return;
        }

        static_buffer buffer;
        detail::format_timestamp_to(buffer);
        fmt::format_to(std::back_inserter(buffer), "[{}] [{}] ",
                       log_process_id_, lookup_level_name(level));

        if(!!(level & log::debug)) {
            fmt::format_to(std::back_inserter(buffer), "<{}():{}> ", func,
                           lineno);
        }

        fmt::format_to(std::back_inserter(buffer), std::forward<Args>(args)...);
        fmt::format_to(std::back_inserter(buffer), "\n");
        detail::log_buffer(log_fd_, buffer);
    }

    inline int
    log(log_level level, const char* fmt, va_list ap) {

        if(!(level & log_mask_)) {
            return 0;
        }

        // we use buffer views to compose the logging messages to
        // avoid copying buffers as much as possible
        struct buffer_view {
            const void* addr;
            std::size_t size;
        };

        // helper lambda to print an iterable of buffer_views
        const auto log_buffer_views = [this](const auto& buffers) {
            std::size_t n = 0;

            for(const auto& bv : buffers) {
                if(bv.addr != nullptr) {
                    detail::log_buffer(log_fd_, bv.addr, bv.size);
                    n += bv.size;
                }
            }

            return n;
        };


        static_buffer prefix;
        detail::format_timestamp_to(prefix);
        fmt::format_to(std::back_inserter(prefix), "[{}] [{}] ",
                       log_process_id_, lookup_level_name(level));

        char buffer[max_buffer_size];
        const int n = vsnprintf(buffer, sizeof(buffer), fmt, ap);

        std::array<buffer_view, 3> buffers{};

        int i = 0;
        int m = 0;
        const char* addr = buffer;
        const char* p = nullptr;
        while((p = std::strstr(addr, "\n")) != nullptr) {
            buffers[0] = buffer_view{prefix.data(), prefix.size()};
            buffers[1] =
                    buffer_view{addr, static_cast<std::size_t>(p - addr) + 1};

            m += log_buffer_views(buffers);
            addr = p + 1;
            ++i;
        }

        // original line might not end with (or include) '\n'
        if(buffer[n - 1] != '\n') {
            buffers[0] = buffer_view{prefix.data(), prefix.size()};
            buffers[1] = buffer_view{
                    addr, static_cast<std::size_t>(&buffer[n] - addr)};
            buffers[2] = buffer_view{"\n", 1};

            m += log_buffer_views(buffers);
        }

        return m;
    }

    template <typename... Args>
    static inline void
    log_message(std::FILE* fp, Args&&... args) {
        log_message(::fileno(fp), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static inline void
    log_message(int fd, Args&&... args) {

        if(fd < 0) {
            throw std::runtime_error("Invalid file descriptor");
        }

        static_buffer buffer;
        fmt::format_to(std::back_inserter(buffer), std::forward<Args>(args)...);
        fmt::format_to(std::back_inserter(buffer), "\n");
        detail::log_buffer(fd, buffer);
    }

    void
    log_syscall(syscall::info info, const long syscall_number,
                const long args[6], std::optional<long> result = {});

    static std::shared_ptr<logger>&
    global_logger() {
        static std::shared_ptr<logger> s_global_logger;
        return s_global_logger;
    }

    int log_fd_;
    int log_process_id_;
    log_level log_mask_;

#ifdef GKFS_DEBUG_BUILD
    std::bitset<512> filtered_syscalls_;
    int debug_verbosity_;
#endif
};

// the following static functions can be used to interact
// with a globally registered logger instance

template <typename... Args>
static inline void
create_global_logger(Args&&... args) {

    auto foo = std::make_shared<logger>(std::forward<Args>(args)...);
    logger::global_logger() = foo;
}

static inline void
register_global_logger(logger&& lg) {
    logger::global_logger() = std::make_shared<logger>(std::move(lg));
}

static inline std::shared_ptr<logger>&
get_global_logger() {
    return logger::global_logger();
}

static inline void
destroy_global_logger() {
    logger::global_logger().reset();
}

} // namespace gkfs::log

#define LOG(XXX, ...) LOG_##XXX(__VA_ARGS__)

#ifndef GKFS_ENABLE_LOGGING

#define LOG_INFO(...)                                                          \
    do {                                                                       \
    } while(0);
#define LOG_WARNING(...)                                                       \
    do {                                                                       \
    } while(0);
#define LOG_ERROR(...)                                                         \
    do {                                                                       \
    } while(0);
#define LOG_CRITICAL(...)                                                      \
    do {                                                                       \
    } while(0);
#define LOG_HERMES(...)                                                        \
    do {                                                                       \
    } while(0);
#define LOG_MERCURY(...)                                                       \
    do {                                                                       \
    } while(0);
#define LOG_SYSCALL(...)                                                       \
    do {                                                                       \
    } while(0);
#define LOG_DEBUG(...)                                                         \
    do {                                                                       \
    } while(0);
#define LOG_TRACE_READS(...)                                                   \
    do {                                                                       \
    } while(0);

#else // !GKFS_ENABLE_LOGGING

#define LOG_INFO(...)                                                          \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(gkfs::log::info, __func__,     \
                                                __LINE__, __VA_ARGS__);        \
        }                                                                      \
    } while(0);

#define LOG_WARNING(...)                                                       \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(gkfs::log::warning, __func__,  \
                                                __LINE__, __VA_ARGS__);        \
        }                                                                      \
    } while(0);

#define LOG_ERROR(...)                                                         \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(gkfs::log::error, __func__,    \
                                                __LINE__, __VA_ARGS__);        \
        }                                                                      \
    } while(0);

#define LOG_CRITICAL(...)                                                      \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(gkfs::log::critical, __func__, \
                                                __LINE__, __VA_ARGS__);        \
        }                                                                      \
    } while(0);

#define LOG_HERMES(...)                                                        \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(gkfs::log::hermes, __func__,   \
                                                __LINE__, __VA_ARGS__);        \
        }                                                                      \
    } while(0);

#define LOG_MERCURY(...)                                                       \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(gkfs::log::mercury, __func__,  \
                                                __LINE__, __VA_ARGS__);        \
        }                                                                      \
    } while(0);

#define LOG_TRACE_READS(...)                                                   \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(                               \
                    gkfs::log::trace_reads, __func__, __LINE__, __VA_ARGS__);  \
        }                                                                      \
    } while(0);

#ifdef GKFS_DEBUG_BUILD

#define LOG_SYSCALL(...)                                                       \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log_syscall(__VA_ARGS__);          \
        }                                                                      \
    } while(0);

#define LOG_DEBUG(...)                                                         \
    do {                                                                       \
        if(gkfs::log::get_global_logger()) {                                   \
            gkfs::log::get_global_logger()->log(gkfs::log::debug, __func__,    \
                                                __LINE__, __VA_ARGS__);        \
        }                                                                      \
    } while(0);

#else // ! GKFS_DEBUG_BUILD

#define LOG_SYSCALL(...)                                                       \
    do {                                                                       \
    } while(0);
#define LOG_DEBUG(...)                                                         \
    do {                                                                       \
    } while(0);

#endif // ! GKFS_DEBUG_BUILD
#endif // !GKFS_ENABLE_LOGGING

#endif // LIBGKFS_LOGGING_HPP
