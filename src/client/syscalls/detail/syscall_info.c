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

// This file uses special C formatting for a better overview
// clang-format off

#define _GNU_SOURCE
#include <syscall.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <client/syscalls/detail/syscall_info.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define SYSCALL(id, nargs, ret, ...) \
    [SYS_##id] =                     \
{                                    \
    .s_nr = SYS_##id,                \
    .s_name = #id,                   \
    .s_nargs = nargs,                \
    .s_return_type = ret,            \
    .s_args = {__VA_ARGS__}          \
}

#define S_NOARGS() {0}

#define S_UARG(t) \
{                 \
    .a_type = t,  \
    .a_name = #t  \
}

#define S_NARG(t, n) \
{                    \
    .a_type = t,     \
    .a_name = n      \
}

#define S_RET(t) \
{                \
    .r_type = t  \
}

/* Linux syscalls on x86_64 */
const struct syscall_info syscall_table[] = {
    SYSCALL(read,                    3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "buf"),              S_NARG(arg, "count")),
    SYSCALL(write,                   3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "buf"),              S_NARG(arg, "count")),
#ifdef SYS_open
    SYSCALL(open,                    2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(open_flags, "flags")),
#endif
    SYSCALL(close,                   1,  S_RET(rdec),    S_UARG(fd)),
#ifdef SYS_stat
    SYSCALL(stat,                    2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(ptr, "statbuf")),
#endif
    SYSCALL(fstat,                   2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "statbuf")),
#ifdef SYS_lstat
    SYSCALL(lstat,                   2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(ptr, "statbuf")),
#endif
#ifdef SYS_poll
    SYSCALL(poll,                    3,  S_RET(rdec),    S_NARG(ptr, "fds"),            S_NARG(dec, "nfds"),             S_NARG(dec, "timeout")),
#endif
    SYSCALL(lseek,                   3,  S_RET(rdec),    S_UARG(fd),                    S_UARG(offset),                  S_UARG(whence)),
    SYSCALL(mmap,                    6,  S_RET(rptr),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(mmap_prot, "prot"),        S_NARG(mmap_flags, "flags"), S_UARG(fd),                  S_UARG(offset)),
    SYSCALL(mprotect,                3,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(mmap_prot, "prot")),
    SYSCALL(munmap,                  2,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length")),
    SYSCALL(brk,                     1,  S_RET(rdec),    S_NARG(ptr, "addr")),
    SYSCALL(rt_sigaction,            4,  S_RET(rdec),    S_NARG(signum, "signum"),      S_NARG(ptr, "act"),              S_NARG(ptr, "oldact"),            S_NARG(dec, "sigsetsize")),
    SYSCALL(rt_sigprocmask,          4,  S_RET(rdec),    S_NARG(sigproc_how, "how"),    S_NARG(ptr, "set"),              S_NARG(ptr, "oldset"),            S_NARG(dec, "sigsetsize")),
    SYSCALL(rt_sigreturn,            0,  S_RET(rnone),   S_NOARGS()),
    SYSCALL(ioctl,                   3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(arg, "cmd"),              S_NARG(arg, "argp")),
    SYSCALL(pread64,                 4,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "buf"),              S_NARG(arg, "count"),             S_UARG(offset)),
    SYSCALL(pwrite64,                4,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "buf"),              S_NARG(arg, "count"),             S_UARG(offset)),
    SYSCALL(readv,                   3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "iov"),              S_NARG(dec, "iovcnt")),
    SYSCALL(writev,                  3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "iov"),              S_NARG(dec, "iovcnt")),
#ifdef SYS_access
    SYSCALL(access,                  2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(octal_mode, "mode")),
#endif
#ifdef SYS_pipe
    SYSCALL(pipe,                    1,  S_RET(rdec),    S_NARG(ptr, "pipefd")),
#endif
#ifdef SYS_select
    SYSCALL(select,                  5,  S_RET(rdec),    S_NARG(dec, "nfds"),           S_NARG(ptr, "readfds"),          S_NARG(ptr, "writefds"),          S_NARG(ptr, "exceptfds"),    S_NARG(ptr, "timeout")),
#endif
    SYSCALL(sched_yield,             0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(mremap,                  5,  S_RET(rdec),    S_NARG(ptr, "old_address"),    S_NARG(dec, "old_size"),         S_NARG(dec, "new_size"),          S_NARG(arg, "flags"),        S_NARG(ptr, "new_addr")),
    SYSCALL(msync,                   3,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(arg, "flags")),
    SYSCALL(mincore,                 3,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(ptr, "vec")),
    SYSCALL(madvise,                 3,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(arg, "behavior")),
#ifdef SYS_semget
    SYSCALL(shmget,                  3,  S_RET(rdec),    S_NARG(arg, "key"),            S_NARG(dec, "size"),             S_NARG(arg, "flag")),
#endif // SYS_semget
#ifdef SYS_shmat
    SYSCALL(shmat,                   3,  S_RET(rdec),    S_NARG(arg, "shmid"),          S_NARG(ptr, "shmaddr"),          S_NARG(arg, "shmflg")),
#endif // SYS_shmat
#ifdef SYS_shmctl
    SYSCALL(shmctl,                  3,  S_RET(rdec),    S_NARG(arg, "shmid"),          S_NARG(arg, "cmd"),              S_NARG(ptr, "buf")),
#endif // SYS_shmctl
    SYSCALL(dup,                     1,  S_RET(rdec),    S_NARG(fd, "oldfd")),
#ifdef SYS_dup2
    SYSCALL(dup2,                    2,  S_RET(rdec),    S_NARG(fd, "oldfd"),           S_NARG(fd, "newfd")),
#endif
#ifdef SYS_pause
    SYSCALL(pause,                   0,  S_RET(rdec),    S_NOARGS()),
#endif
    SYSCALL(nanosleep,               2,  S_RET(rdec),    S_NARG(ptr, "rqtp"),           S_NARG(ptr, "rmtp")),
    SYSCALL(getitimer,               2,  S_RET(rdec),    S_NARG(arg, "which"),          S_NARG(ptr, "value")),
#ifdef SYS_alarm
    SYSCALL(alarm,                   1,  S_RET(rdec),    S_NARG(dec, "seconds")),
#endif
    SYSCALL(setitimer,               3,  S_RET(rdec),    S_NARG(arg, "which"),          S_NARG(ptr, "value"),            S_NARG(ptr, "ovalue")),
    SYSCALL(getpid,                  0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(sendfile,                4,  S_RET(rdec),    S_NARG(fd, "out_fd"),          S_NARG(fd, "in_fd"),             S_NARG(ptr, "offset"),            S_NARG(arg, "count")),
    SYSCALL(socket,                  3,  S_RET(rdec),    S_NARG(arg, "domain"),         S_NARG(arg, "type"),             S_NARG(arg, "protocol")),
    SYSCALL(connect,                 3,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "addr"),             S_NARG(arg, "addrlen")),
    SYSCALL(accept,                  3,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "addr"),             S_NARG(ptr, "addrlen")),
    SYSCALL(sendto,                  5,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "dest_addr"),        S_NARG(arg, "len"),               S_NARG(ptr, "addr"),         S_NARG(arg, "addrlen")),
    SYSCALL(recvfrom,                5,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "src_addr"),         S_NARG(arg, "len"),               S_NARG(ptr, "addr"),         S_NARG(ptr, "addrlen")),
    SYSCALL(sendmsg,                 3,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "msg"),              S_NARG(arg, "flags")),
    SYSCALL(recvmsg,                 3,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "msg"),              S_NARG(arg, "flags")),
    SYSCALL(shutdown,                2,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(arg, "how")),
    SYSCALL(bind,                    3,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "addr"),             S_NARG(arg, "addrlen")),
    SYSCALL(listen,                  2,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(arg, "backlog")),
    SYSCALL(getsockname,             3,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "addr"),             S_NARG(ptr, "addrlen")),
    SYSCALL(getpeername,             3,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "addr"),             S_NARG(ptr, "addrlen")),
    SYSCALL(socketpair,              4,  S_RET(rdec),    S_NARG(arg, "domain"),         S_NARG(arg, "type"),             S_NARG(arg, "protocol"),          S_NARG(ptr, "sv")),
    SYSCALL(setsockopt,              5,  S_RET(rdec),    S_UARG(fd),                    S_NARG(arg, "level"),            S_NARG(arg, "optname"),           S_NARG(ptr, "optval"),       S_NARG(arg, "optlen")),
    SYSCALL(getsockopt,              5,  S_RET(rdec),    S_UARG(fd),                    S_NARG(arg, "level"),            S_NARG(arg, "optname"),           S_NARG(ptr, "optval"),       S_NARG(ptr, "optlen")),
    SYSCALL(clone,                   5,  S_RET(rdec),    S_NARG(clone_flags, "flags"),  S_NARG(ptr, "child_stack"),      S_NARG(ptr, "ptid"),              S_NARG(ptr, "ctid"),         S_NARG(ptr, "newtls")),
#ifdef SYS_fork
    SYSCALL(fork,                    0,  S_RET(rdec),    S_NOARGS()),
#endif
#ifdef SYS_vfork
    SYSCALL(vfork,                   0,  S_RET(rdec),    S_NOARGS()),
#endif
    SYSCALL(execve,                  3,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(ptr, "argv"),             S_NARG(ptr, "envp")),
    SYSCALL(exit,                    1,  S_RET(rnone),   S_NARG(dec, "status")),
    SYSCALL(wait4,                   4,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "stat_addr"),        S_NARG(arg, "options"),           S_NARG(ptr, "rusage")),
    SYSCALL(kill,                    2,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(signum, "sig")),
    SYSCALL(uname,                   1,  S_RET(rdec),    S_NARG(ptr, "buf")),
#ifdef SYS_semget
    SYSCALL(semget,                  3,  S_RET(rdec),    S_NARG(arg, "key"),            S_NARG(dec, "nsems"),            S_NARG(arg, "semflg")),
#endif // SYS_semop
#ifdef SYS_semop
    SYSCALL(semop,                   3,  S_RET(rdec),    S_NARG(dec, "semid"),          S_NARG(ptr, "sops"),             S_NARG(arg, "nsops")),
#endif // SYS_semop
#ifdef SYS_semctl
    SYSCALL(semctl,                  4,  S_RET(rdec),    S_NARG(dec, "semid"),          S_NARG(dec, "semnum"),           S_NARG(arg, "cmd"),               S_NARG(arg, "arg")),
#endif // SYS_semctl
#ifdef SYS_shmdt
    SYSCALL(shmdt,                   1,  S_RET(rdec),    S_NARG(ptr, "shmaddr")),
#endif // SYS_shmdt
#ifdef SYS_msgget
    SYSCALL(msgget,                  2,  S_RET(rdec),    S_NARG(arg, "key"),            S_NARG(arg, "msflg")),
#endif // SYS_msgget
#ifdef SYS_msgsnd
    SYSCALL(msgsnd,                  4,  S_RET(rdec),    S_NARG(arg, "msqid"),          S_NARG(ptr, "msgp"),             S_NARG(dec, "msgsz"),             S_NARG(arg, "msflg")),
#endif // SYS_msgsnd
#ifdef SYS_msgrcv
    SYSCALL(msgrcv,                  5,  S_RET(rdec),    S_NARG(arg, "msqid"),          S_NARG(ptr, "msgp"),             S_NARG(dec, "msgsz"),             S_NARG(arg, "msgtyp"),       S_NARG(arg, "msflg")),
#endif // SYS_msgrcv
#ifdef SYS_msgctl
    SYSCALL(msgctl,                  3,  S_RET(rdec),    S_NARG(arg, "msqid"),          S_NARG(arg, "cmd"),              S_NARG(ptr, "buf")),
#endif // SYS_msgctl
    SYSCALL(fcntl,                   3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(arg, "cmd"),              S_NARG(arg, "arg")),
    SYSCALL(flock,                   2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(arg, "cmd")),
    SYSCALL(fsync,                   1,  S_RET(rdec),    S_UARG(fd)),
    SYSCALL(fdatasync,               2,  S_RET(rdec),    S_UARG(fd)),
    SYSCALL(truncate,                2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(arg, "length")),
    SYSCALL(ftruncate,               2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(offset, "length")),
#ifdef SYS_getdents
    SYSCALL(getdents,                3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "dirent"),           S_NARG(arg, "count")),
#endif
    SYSCALL(getcwd,                  2,  S_RET(rdec),    S_NARG(ptr, "buf"),            S_NARG(dec, "size")),
    SYSCALL(chdir,                   1,  S_RET(rdec),    S_NARG(cstr, "pathname")),
    SYSCALL(fchdir,                  1,  S_RET(rdec),    S_UARG(fd)),
#ifdef SYS_rename
    SYSCALL(rename,                  2,  S_RET(rdec),    S_NARG(cstr, "oldpath"),       S_NARG(cstr, "newpath")),
#endif
#ifdef SYS_mkdir
    SYSCALL(mkdir,                   2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(octal_mode, "mode")),
#endif
#ifdef SYS_rmdir
    SYSCALL(rmdir,                   1,  S_RET(rdec),    S_NARG(cstr, "pathname")),
#endif
#ifdef SYS_creat
    SYSCALL(creat,                   2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(octal_mode, "mode")),
#endif
#ifdef SYS_link
    SYSCALL(link,                    2,  S_RET(rdec),    S_NARG(cstr, "oldpath"),       S_NARG(cstr, "newpath")),
#endif
#ifdef SYS_unlink
    SYSCALL(unlink,                  1,  S_RET(rdec),    S_NARG(cstr, "pathname")),
#endif
#ifdef SYS_symlink
    SYSCALL(symlink,                 2,  S_RET(rdec),    S_NARG(cstr, "target"),        S_NARG(cstr, "linkpath")),
#endif
#ifdef SYS_readlink
    SYSCALL(readlink,                2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(ptr, "buf"),              S_NARG(arg, "bufsiz")),
#endif
#ifdef SYS_chmod
    SYSCALL(chmod,                   2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(octal_mode, "mode")),
#endif
    SYSCALL(fchmod,                  2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(octal_mode, "mode")),
#ifdef SYS_chown
    SYSCALL(chown,                   3,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(dec, "user"),             S_NARG(dec, "group")),
#endif
    SYSCALL(fchown,                  3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(dec, "user"),             S_NARG(dec, "group")),
#ifdef SYS_lchown
    SYSCALL(lchown,                  3,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(dec, "user"),             S_NARG(dec, "group")),
#endif
    SYSCALL(umask,                   1,  S_RET(rdec),    S_NARG(arg, "mask")),
    SYSCALL(gettimeofday,            2,  S_RET(rdec),    S_NARG(ptr, "tv"),             S_NARG(ptr, "tz")),
    SYSCALL(getrlimit,               2,  S_RET(rdec),    S_NARG(arg, "resource"),       S_NARG(ptr, "rlim")),
    SYSCALL(getrusage,               2,  S_RET(rdec),    S_NARG(arg, "who"),            S_NARG(ptr, "ru")),
    SYSCALL(sysinfo,                 1,  S_RET(rdec),    S_NARG(ptr, "info")),
    SYSCALL(times,                   1,  S_RET(rdec),    S_NARG(ptr, "tbuf")),
    SYSCALL(ptrace,                  4,  S_RET(rdec),    S_NARG(arg, "request"),        S_NARG(dec, "pid"),              S_NARG(ptr, "addr"),              S_NARG(ptr, "data")),
    SYSCALL(getuid,                  0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(syslog,                  3,  S_RET(rdec),    S_NARG(arg, "type"),           S_NARG(ptr, "buf"),              S_NARG(arg, "length")),
    SYSCALL(getgid,                  0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(setuid,                  1,  S_RET(rdec),    S_NARG(dec, "uid")),
    SYSCALL(setgid,                  1,  S_RET(rdec),    S_NARG(dec, "gid")),
    SYSCALL(geteuid,                 0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(getegid,                 0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(setpgid,                 2,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(dec, "pgid")),
    SYSCALL(getppid,                 0,  S_RET(rdec),    S_NOARGS()),
#ifdef SYS_getpgrp
    SYSCALL(getpgrp,                 0,  S_RET(rdec),    S_NOARGS()),
#endif
    SYSCALL(setsid,                  0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(setreuid,                2,  S_RET(rdec),    S_NARG(dec, "ruid"),           S_NARG(dec, "euid")),
    SYSCALL(setregid,                2,  S_RET(rdec),    S_NARG(dec, "rgid"),           S_NARG(dec, "egid")),
    SYSCALL(getgroups,               2,  S_RET(rdec),    S_NARG(arg, "gidsetsize"),     S_NARG(ptr, "grouplist")),
    SYSCALL(setgroups,               2,  S_RET(rdec),    S_NARG(arg, "gidsetsize"),     S_NARG(ptr, "grouplist")),
    SYSCALL(setresuid,               3,  S_RET(rdec),    S_NARG(dec, "ruid"),           S_NARG(dec, "euid"),             S_NARG(dec, "suid")),
    SYSCALL(getresuid,               3,  S_RET(rdec),    S_NARG(ptr, "ruid"),           S_NARG(ptr, "euid"),             S_NARG(ptr, "suid")),
    SYSCALL(setresgid,               3,  S_RET(rdec),    S_NARG(dec, "rgid"),           S_NARG(dec, "egid"),             S_NARG(dec, "sgid")),
    SYSCALL(getresgid,               3,  S_RET(rdec),    S_NARG(ptr, "rgid"),           S_NARG(ptr, "egid"),             S_NARG(ptr, "sgid")),
    SYSCALL(getpgid,                 1,  S_RET(rdec),    S_NARG(dec, "pid")),
    SYSCALL(setfsuid,                1,  S_RET(rdec),    S_NARG(dec, "uid")),
    SYSCALL(setfsgid,                1,  S_RET(rdec),    S_NARG(dec, "gid")),
    SYSCALL(getsid,                  1,  S_RET(rdec),    S_NARG(dec, "pid")),
    SYSCALL(capget,                  2,  S_RET(rdec),    S_NARG(ptr, "header"),         S_NARG(ptr, "datap")),
    SYSCALL(capset,                  2,  S_RET(rdec),    S_NARG(ptr, "header"),         S_NARG(ptr, "datap")),
    SYSCALL(rt_sigpending,           2,  S_RET(rdec),    S_NARG(ptr, "set"),            S_NARG(dec, "sigsetsize")),
    SYSCALL(rt_sigtimedwait,         4,  S_RET(rdec),    S_NARG(ptr, "uthese"),         S_NARG(ptr, "uinfo"),            S_NARG(ptr, "uts"),               S_NARG(dec, "sigsetsize")),
    SYSCALL(rt_sigqueueinfo,         4,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(signum, "sig"),           S_NARG(ptr, "uinfo")),
    SYSCALL(rt_sigsuspend,           2,  S_RET(rdec),    S_NARG(ptr, "unewset"),        S_NARG(dec, "sigsetsize")),
    SYSCALL(sigaltstack,             2,  S_RET(rdec),    S_NARG(ptr, "ss"),             S_NARG(ptr, "old_ss")),
#ifdef SYS_utime
    SYSCALL(utime,                   2,  S_RET(rdec),    S_NARG(cstr, "filename"),      S_NARG(ptr, "times")),
#endif
#ifdef SYS_mknod
    SYSCALL(mknod,                   3,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(octal_mode, "mode"),      S_NARG(arg, "dev")),
#endif
#ifdef SYS_uselib
    SYSCALL(uselib,                  1,  S_RET(rdec),    S_NARG(cstr, "library")),
#endif
    SYSCALL(personality,             1,  S_RET(rdec),    S_NARG(arg, "personality")),
#ifdef SYS_ustat
    SYSCALL(ustat,                   2,  S_RET(rdec),    S_NARG(arg, "dev"),            S_NARG(ptr, "ubuf")),
#endif
    SYSCALL(statfs,                  2,  S_RET(rdec),    S_NARG(cstr, "path"),          S_NARG(ptr, "buf")),
    SYSCALL(fstatfs,                 2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "buf")),
#ifdef SYS_sysfs
    SYSCALL(sysfs,                   3,  S_RET(rdec),    S_NARG(arg, "option"),         S_NARG(ptr, "arg1"),             S_NARG(ptr, "arg2")),
#endif
    SYSCALL(getpriority,             2,  S_RET(rdec),    S_NARG(arg, "which"),          S_NARG(arg, "who")),
    SYSCALL(setpriority,             3,  S_RET(rdec),    S_NARG(arg, "which"),          S_NARG(arg, "who"),              S_NARG(arg, "prio")),
    SYSCALL(sched_setparam,          2,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "param")),
    SYSCALL(sched_getparam,          2,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "param")),
    SYSCALL(sched_setscheduler,      3,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(arg, "policy"),           S_NARG(ptr, "param")),
    SYSCALL(sched_getscheduler,      1,  S_RET(rdec),    S_NARG(dec, "pid")),
    SYSCALL(sched_get_priority_max,  1,  S_RET(rdec),    S_NARG(arg, "policy")),
    SYSCALL(sched_get_priority_min,  1,  S_RET(rdec),    S_NARG(arg, "policy")),
    SYSCALL(sched_rr_get_interval,   2,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "interval")),
    SYSCALL(mlock,                   2,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length")),
    SYSCALL(munlock,                 2,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length")),
    SYSCALL(mlockall,                1,  S_RET(rdec),    S_NARG(arg, "flags")),
    SYSCALL(munlockall,              0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(vhangup,                 0,  S_RET(rdec),    S_NOARGS()),
#ifdef SYS_modify_ldt
    SYSCALL(modify_ldt,              3,  S_RET(rdec),    S_NARG(arg, "func"),           S_NARG(ptr, "ptr"),              S_NARG(arg, "bytecount")),
#endif
    SYSCALL(pivot_root,              2,  S_RET(rdec),    S_NARG(cstr, "new_root"),      S_NARG(cstr, "put_old")),
#ifdef SYS_sysctl
    SYSCALL(_sysctl,                 1,  S_RET(rdec),    S_NARG(ptr, "args")),
#endif
    SYSCALL(prctl,                   5,  S_RET(rdec),    S_NARG(arg, "option"),         S_NARG(arg, "arg2"),             S_NARG(arg, "arg3"),              S_NARG(arg, "arg4"),         S_NARG(arg, "arg5")),
#ifdef SYS_arch_prctl
    SYSCALL(arch_prctl,              2,  S_RET(rdec),    S_NARG(arg, "code"),           S_NARG(arg, "addr")),
#endif // SYS_arch_prctl
    SYSCALL(adjtimex,                1,  S_RET(rdec),    S_NARG(ptr, "txc_p")),
    SYSCALL(setrlimit,               2,  S_RET(rdec),    S_NARG(arg, "resource"),       S_NARG(ptr, "rlim")),
    SYSCALL(chroot,                  1,  S_RET(rdec),    S_NARG(cstr, "pathname")),
    SYSCALL(sync,                    0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(acct,                    1,  S_RET(rdec),    S_NARG(cstr, "pathname")),
    SYSCALL(settimeofday,            2,  S_RET(rdec),    S_NARG(ptr, "tv"),             S_NARG(ptr, "tz")),
    SYSCALL(mount,                   5,  S_RET(rdec),    S_NARG(cstr, "dev_name"),      S_NARG(cstr, "dir_name"),        S_NARG(cstr, "type"),             S_NARG(arg, "flags"),        S_NARG(ptr, "data")),
    SYSCALL(umount2,                 2,  S_RET(rdec),    S_NARG(cstr, "target"),        S_NARG(arg, "flags")),
    SYSCALL(swapon,                  2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(arg, "swap_flags")),
    SYSCALL(swapoff,                 1,  S_RET(rdec),    S_NARG(cstr, "pathname")),
    SYSCALL(reboot,                  4,  S_RET(rdec),    S_NARG(arg, "magic1"),         S_NARG(arg, "magic2"),           S_NARG(arg, "cmd"),               S_NARG(ptr, "arg")),
    SYSCALL(sethostname,             2,  S_RET(rdec),    S_NARG(cstr, "name"),          S_NARG(arg, "length")),
    SYSCALL(setdomainname,           2,  S_RET(rdec),    S_NARG(cstr, "name"),          S_NARG(arg, "length")),
#ifdef SYS_iopl
    SYSCALL(iopl,                    1,  S_RET(rdec),    S_NARG(arg, "level")),
#endif
#ifdef SYS_ioperm
    SYSCALL(ioperm,                  3,  S_RET(rdec),    S_NARG(arg, "from"),           S_NARG(arg, "num"),              S_NARG(arg, "on")),
#endif
#ifdef SYS_create_module
    SYSCALL(create_module,           2,  S_RET(rdec),    S_NARG(cstr, "name"),          S_NARG(arg, "size")),
#endif
    SYSCALL(init_module,             3,  S_RET(rdec),    S_NARG(ptr, "module_image"),   S_NARG(dec, "length"),           S_NARG(cstr, "param_values")),
    SYSCALL(delete_module,           2,  S_RET(rdec),    S_NARG(cstr, "name"),          S_NARG(arg, "flags")),
#ifdef SYS_get_kernel_syms
    SYSCALL(get_kernel_syms,         1,  S_RET(rdec),    S_NARG(ptr, "table")),
#endif
#ifdef SYS_query_module
    SYSCALL(query_module,            5,  S_RET(rdec),    S_NARG(cstr, "name"),          S_NARG(arg, "which"),            S_NARG(ptr, "buf"),               S_NARG(arg, "bufsize"),      S_NARG(ptr, "ret")),
#endif
#ifdef SYS_quotactl
    SYSCALL(quotactl,                4,  S_RET(rdec),    S_NARG(arg, "cmd"),            S_NARG(cstr, "special"),         S_NARG(arg, "id"),                S_NARG(ptr, "addr")),
#endif
    SYSCALL(nfsservctl,              3,  S_RET(rdec),    S_NARG(arg, "cmd"),            S_NARG(ptr, "argp"),             S_NARG(ptr, "resp")),
#ifdef SYS_getpmsg    
SYSCALL(getpmsg,                 5,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2"),              S_NARG(arg, "arg3"),         S_NARG(arg, "arg4")),
#endif
#ifdef SYS_putmsg
    SYSCALL(putpmsg,                 5,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2"),              S_NARG(arg, "arg3"),         S_NARG(arg, "arg4")),
#endif
#ifdef SYS_afs_syscall
    SYSCALL(afs_syscall,             5,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2"),              S_NARG(arg, "arg3"),         S_NARG(arg, "arg4")),
#endif
#ifdef SYS_tuxcall
    SYSCALL(tuxcall,                 3,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2")),
#endif
#ifdef SYS_security
    SYSCALL(security,                3,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2")),
#endif // SYS_security
    SYSCALL(gettid,                  0,  S_RET(rdec),    S_NOARGS()),
    SYSCALL(readahead,               3,  S_RET(rdec),    S_UARG(fd),                    S_UARG(offset),                  S_NARG(arg, "count")),
    SYSCALL(setxattr,                5,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(cstr, "pathname"),        S_NARG(ptr, "value"),             S_NARG(dec, "size"),         S_NARG(arg, "flags")),
    SYSCALL(lsetxattr,               5,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(cstr, "pathname"),        S_NARG(ptr, "value"),             S_NARG(dec, "size"),         S_NARG(arg, "flags")),
    SYSCALL(fsetxattr,               5,  S_RET(rdec),    S_UARG(fd),                    S_NARG(cstr, "pathname"),        S_NARG(ptr, "value"),             S_NARG(dec, "size"),         S_NARG(arg, "flags")),
    SYSCALL(getxattr,                4,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(cstr, "pathname"),        S_NARG(ptr, "value"),             S_NARG(dec, "size")),
    SYSCALL(lgetxattr,               4,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(cstr, "pathname"),        S_NARG(ptr, "value"),             S_NARG(dec, "size")),
    SYSCALL(fgetxattr,               4,  S_RET(rdec),    S_UARG(fd),                    S_NARG(cstr, "pathname"),        S_NARG(ptr, "value"),             S_NARG(dec, "size")),
    SYSCALL(listxattr,               3,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(ptr, "list"),             S_NARG(dec, "size")),
    SYSCALL(llistxattr,              3,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(ptr, "list"),             S_NARG(dec, "size")),
    SYSCALL(flistxattr,              3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "list"),             S_NARG(dec, "size")),
    SYSCALL(removexattr,             2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(cstr, "pathname")),
    SYSCALL(lremovexattr,            2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(cstr, "pathname")),
    SYSCALL(fremovexattr,            2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(cstr, "pathname")),
    SYSCALL(tkill,                   2,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(signum, "sig")),
#ifdef SYS_time
    SYSCALL(time,                    1,  S_RET(rdec),    S_NARG(ptr, "tloc")),
#endif
    SYSCALL(futex,                   6,  S_RET(rdec),    S_NARG(ptr, "uaddr"),          S_NARG(arg, "op"),               S_NARG(arg, "val"),               S_NARG(ptr, "utime"),        S_NARG(ptr, "uaddr2"),       S_NARG(arg, "val3")),
    SYSCALL(sched_setaffinity,       3,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(arg, "length"),           S_NARG(ptr, "mask")),
    SYSCALL(sched_getaffinity,       3,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(arg, "length"),           S_NARG(ptr, "mask")),
#ifdef SYS_set_thread_area
    SYSCALL(set_thread_area,         1,  S_RET(rdec),    S_NARG(ptr, "u_info")),
#endif // SYS_set_thread_area
    SYSCALL(io_setup,                2,  S_RET(rdec),    S_NARG(dec, "nr_reqs"),        S_NARG(ptr, "ctx")),
    SYSCALL(io_destroy,              1,  S_RET(rdec),    S_NARG(ptr, "ctx")),
    SYSCALL(io_getevents,            5,  S_RET(rdec),    S_NARG(ptr, "ctx_id"),         S_NARG(dec, "min_nr"),           S_NARG(dec, "nr"),                S_NARG(ptr, "events"),       S_NARG(ptr, "timeout")),
    SYSCALL(io_submit,               3,  S_RET(rdec),    S_NARG(ptr, "ctx_id"),         S_NARG(dec, "nr"),               S_NARG(ptr, "iocbpp")),
    SYSCALL(io_cancel,               3,  S_RET(rdec),    S_NARG(ptr, "ctx_id"),         S_NARG(ptr, "iocb"),             S_NARG(ptr, "result")),
#ifdef SYS_get_thread_area
    SYSCALL(get_thread_area,         1,  S_RET(rdec),    S_NARG(ptr, "u_info")),
#endif // SYS_get_thread_area
    SYSCALL(lookup_dcookie,          3,  S_RET(rdec),    S_NARG(arg, "cookie64"),       S_NARG(ptr, "buf"),              S_NARG(dec, "length")),
#ifdef SYS_epoll_create
    SYSCALL(epoll_create,            3,  S_RET(rdec),    S_NARG(arg, "size")),
#endif
#ifdef SYS_epoll_ctl_old
    SYSCALL(epoll_ctl_old,           4,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2"),              S_NARG(arg, "arg3")),
#endif // SYS_epoll_ctl_old
#ifdef SYS_epoll_wait_old
    SYSCALL(epoll_wait_old,          4,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2"),              S_NARG(arg, "arg3")),
#endif // SYS_epoll_wait_old
    SYSCALL(remap_file_pages,        5,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "size"),             S_NARG(mmap_prot, "prot"),        S_NARG(dec, "pgoff"),        S_NARG(arg, "flags")),
    SYSCALL(getdents64,              3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "dirent"),           S_NARG(arg, "count")),
    SYSCALL(set_tid_address,         1,  S_RET(rdec),    S_NARG(ptr, "tidptr")),
    SYSCALL(restart_syscall,         0,  S_RET(rdec),    S_NOARGS()),
#ifdef SYS_semtimedop
    SYSCALL(semtimedop,              4,  S_RET(rdec),    S_NARG(dec, "semid"),          S_NARG(ptr, "sops"),             S_NARG(arg, "nsops"),             S_NARG(ptr, "timeout")),
#endif // SYS_semtimedop
    SYSCALL(fadvise64,               4,  S_RET(rdec),    S_UARG(fd),                    S_UARG(offset),                  S_NARG(dec, "length"),            S_NARG(arg, "advice")),
    SYSCALL(timer_create,            3,  S_RET(rdec),    S_NARG(arg, "which_clock"),    S_NARG(ptr, "timer_event_spec"), S_NARG(ptr, "created_timer_id")),
    SYSCALL(timer_settime,           4,  S_RET(rdec),    S_NARG(arg, "timer_id"),       S_NARG(arg, "flags"),            S_NARG(ptr, "new_setting"),       S_NARG(ptr, "old_setting")),
    SYSCALL(timer_gettime,           2,  S_RET(rdec),    S_NARG(arg, "timer_id"),       S_NARG(ptr, "setting")),
    SYSCALL(timer_getoverrun,        1,  S_RET(rdec),    S_NARG(arg, "timer_id")),
    SYSCALL(timer_delete,            1,  S_RET(rdec),    S_NARG(arg, "timer_id")),
    SYSCALL(clock_settime,           2,  S_RET(rdec),    S_NARG(arg, "which_clock"),    S_NARG(ptr, "tp")),
    SYSCALL(clock_gettime,           2,  S_RET(rdec),    S_NARG(arg, "which_clock"),    S_NARG(ptr, "tp")),
    SYSCALL(clock_getres,            2,  S_RET(rdec),    S_NARG(arg, "which_clock"),    S_NARG(ptr, "tp")),
    SYSCALL(clock_nanosleep,         4,  S_RET(rdec),    S_NARG(arg, "which_clock"),    S_NARG(arg, "flags"),            S_NARG(ptr, "rqtp"),              S_NARG(ptr, "rmtp")),
    SYSCALL(exit_group,              1,  S_RET(rnone),   S_NARG(dec, "status")),
#ifdef SYS_epoll_wait
    SYSCALL(epoll_wait,              4,  S_RET(rdec),    S_NARG(dec, "epfd"),           S_NARG(ptr, "events"),           S_NARG(dec, "maxevents"),         S_NARG(dec32, "timeout")),
#endif
    SYSCALL(epoll_ctl,               4,  S_RET(rdec),    S_NARG(dec, "epfd"),           S_NARG(arg, "op"),               S_UARG(fd),                       S_NARG(ptr, "event")),
    SYSCALL(tgkill,                  3,  S_RET(rdec),    S_NARG(arg, "tgid"),           S_NARG(dec, "pid"),              S_NARG(signum, "sig")),
#ifdef SYS_utimes
    SYSCALL(utimes,                  2,  S_RET(rdec),    S_NARG(cstr, "filename"),      S_NARG(ptr, "utimes")),
#endif
#ifdef SYS_vserver
    SYSCALL(vserver,                 5,  S_RET(rdec),    S_NARG(arg, "arg0"),           S_NARG(arg, "arg1"),             S_NARG(arg, "arg2"),              S_NARG(arg, "arg3"),         S_NARG(arg, "arg4")),
#endif // SYS_vserver
    SYSCALL(mbind,                   6,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(octal_mode, "mode"),       S_NARG(ptr, "nmask"),        S_NARG(arg, "maxnode"),      S_NARG(arg, "flags")),
    SYSCALL(set_mempolicy,           3,  S_RET(rdec),    S_NARG(octal_mode, "mode"),    S_NARG(ptr, "nmask"),            S_NARG(arg, "maxnode")),
    SYSCALL(get_mempolicy,           5,  S_RET(rdec),    S_NARG(ptr, "policy"),         S_NARG(ptr, "nmask"),            S_NARG(arg, "maxnode"),           S_NARG(ptr, "addr"),         S_NARG(arg, "flags")),
    SYSCALL(mq_open,                 4,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(open_flags, "oflag"),     S_NARG(octal_mode, "mode"),       S_NARG(ptr, "attr")),
    SYSCALL(mq_unlink,               1,  S_RET(rdec),    S_NARG(cstr, "pathname")),
    SYSCALL(mq_timedsend,            5,  S_RET(rdec),    S_NARG(arg, "mqdes"),          S_NARG(cstr, "msg_ptr"),         S_NARG(dec, "msg_len"),           S_NARG(arg, "msg_prio"),     S_NARG(ptr, "abs_timeout")),
    SYSCALL(mq_timedreceive,         5,  S_RET(rdec),    S_NARG(arg, "mqdes"),          S_NARG(ptr, "msg_ptr"),          S_NARG(dec, "msg_len"),           S_NARG(ptr, "msg_prio"),     S_NARG(ptr, "abs_timeout")),
    SYSCALL(mq_notify,               2,  S_RET(rdec),    S_NARG(arg, "mqdes"),          S_NARG(ptr, "notification")),
    SYSCALL(mq_getsetattr,           3,  S_RET(rdec),    S_NARG(arg, "mqdes"),          S_NARG(ptr, "mqstat"),           S_NARG(ptr, "omqstat")),
    SYSCALL(kexec_load,              4,  S_RET(rdec),    S_NARG(arg, "entry"),          S_NARG(arg, "nr_segments"),      S_NARG(ptr, "segments"),          S_NARG(arg, "flags")),
    SYSCALL(waitid,                  5,  S_RET(rdec),    S_NARG(arg, "which"),          S_NARG(dec, "pid"),              S_NARG(ptr, "infop"),             S_NARG(arg, "options"),      S_NARG(ptr, "ru")),
    SYSCALL(add_key,                 5,  S_RET(rdec),    S_NARG(cstr, "type"),          S_NARG(cstr, "description"),     S_NARG(ptr, "payload"),           S_NARG(dec, "plen"),         S_NARG(arg, "destringid")),
    SYSCALL(request_key,             4,  S_RET(rdec),    S_NARG(cstr, "type"),          S_NARG(cstr, "description"),     S_NARG(cstr, "callout_info"),     S_NARG(arg, "destringid")),
    SYSCALL(keyctl,                  5,  S_RET(rdec),    S_NARG(arg, "cmd"),            S_NARG(arg, "arg2"),             S_NARG(arg, "arg3"),              S_NARG(arg, "arg4"),         S_NARG(arg, "arg5")),
    SYSCALL(ioprio_set,              3,  S_RET(rdec),    S_NARG(arg, "which"),          S_NARG(arg, "who"),              S_NARG(dec, "ioprio")),
    SYSCALL(ioprio_get,              2,  S_RET(rdec),    S_NARG(arg, "which"),          S_NARG(arg, "who")),
#ifdef SYS_inotify_init
    SYSCALL(inotify_init,            0,  S_RET(rdec),    S_NOARGS()),
#endif
    SYSCALL(inotify_add_watch,       3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(cstr, "pathname"),        S_NARG(arg, "mask")),
    SYSCALL(inotify_rm_watch,        2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(dec, "wd")),
    SYSCALL(migrate_pages,           4,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(arg, "maxnode"),          S_NARG(ptr, "from"),              S_NARG(ptr, "to")),
    SYSCALL(openat,                  3,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(open_flags, "flags")),
    SYSCALL(mkdirat,                 3,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(octal_mode, "mode")),
    SYSCALL(mknodat,                 4,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "filename"),        S_NARG(octal_mode, "mode"),       S_NARG(arg, "dev")),
    SYSCALL(fchownat,                5,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(dec, "user"),              S_NARG(dec, "group"),        S_NARG(arg, "flag")),
#ifdef SYS_futimesat
    SYSCALL(futimesat,               3,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(ptr, "utimes")),
#endif
    SYSCALL(newfstatat,              4,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(ptr, "statbuf"),           S_NARG(arg, "flag")),
    SYSCALL(unlinkat,                3,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(arg, "flag")),
    SYSCALL(renameat,                4,  S_RET(rdec),    S_NARG(atfd, "olddfd"),        S_NARG(cstr, "oldname"),         S_NARG(atfd, "newdfd"),           S_NARG(cstr, "newname")),
    SYSCALL(linkat,                  5,  S_RET(rdec),    S_NARG(atfd, "olddfd"),        S_NARG(cstr, "oldpath"),         S_NARG(atfd, "newdfd"),           S_NARG(cstr, "newpath"),     S_NARG(arg, "flags")),
    SYSCALL(symlinkat,               3,  S_RET(rdec),    S_NARG(cstr, "oldname"),       S_NARG(atfd, "newdfd"),          S_NARG(cstr, "newname")),
    SYSCALL(readlinkat,              4,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(ptr, "buf"),               S_NARG(arg, "bufsiz")),
    SYSCALL(fchmodat,                3,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "filename"),        S_NARG(octal_mode, "mode")),
    SYSCALL(faccessat,               3,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(octal_mode, "mode")),
#ifdef SYS_faccessat2
    SYSCALL(faccessat2,              4,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(octal_mode, "mode"),       S_NARG(arg, "flags")),
#endif
    SYSCALL(pselect6,                6,  S_RET(rdec),    S_NARG(dec, "nfds"),           S_NARG(ptr, "readfds"),          S_NARG(ptr, "writefds"),          S_NARG(ptr, "exceptfds"),    S_NARG(ptr, "timeval"),      S_NARG(ptr, "sigmask")),
    SYSCALL(ppoll,                   5,  S_RET(rdec),    S_NARG(ptr, "fds"),            S_NARG(dec, "nfds"),             S_NARG(ptr, "tmo_p"),             S_NARG(ptr, "sigmask"),      S_NARG(dec, "sigsetsize")),
    SYSCALL(unshare,                 1,  S_RET(rdec),    S_NARG(arg, "unshare_flags")),
    SYSCALL(set_robust_list,         2,  S_RET(rdec),    S_NARG(ptr, "head"),           S_NARG(dec, "length")),
    SYSCALL(get_robust_list,         3,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "head_ptr"),         S_NARG(ptr, "len_ptr")),
    SYSCALL(splice,                  6,  S_RET(rdec),    S_NARG(dec, "fd_in"),          S_NARG(ptr, "off_in"),           S_NARG(dec, "fd_out"),            S_NARG(ptr, "off_out"),      S_NARG(dec, "length"),       S_NARG(arg, "flags")),
    SYSCALL(tee,                     4,  S_RET(rdec),    S_NARG(dec, "fd_in"),          S_NARG(dec, "fd_out"),           S_NARG(dec, "length"),            S_NARG(arg, "flags")),
#ifdef SYS_sync_file_range
    SYSCALL(sync_file_range,         4,  S_RET(rdec),    S_UARG(fd),                    S_UARG(offset),                  S_NARG(offset, "nbytes"),          S_NARG(arg, "flags")),
#endif // SYS_sync_file_range
    SYSCALL(vmsplice,                4,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "iov"),              S_NARG(arg, "nr_segs"),           S_NARG(arg, "flags")),
    SYSCALL(move_pages,              6,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(arg, "nr_pages"),         S_NARG(ptr, "pages"),             S_NARG(ptr, "nodes"),        S_NARG(ptr, "status"),       S_NARG(arg, "flags")),
    SYSCALL(utimensat,               4,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(ptr, "utimes"),            S_NARG(arg, "flags")),
    SYSCALL(epoll_pwait,             6,  S_RET(rdec),    S_NARG(fd, "epfd"),            S_NARG(ptr, "events"),           S_NARG(dec, "maxevents"),         S_NARG(dec, "timeout"),      S_NARG(ptr, "sigmask"),      S_NARG(dec, "sigsetsize")),
#ifdef SYS_signalfd
    SYSCALL(signalfd,                3,  S_RET(rdec),    S_NARG(dec, "ufd"),            S_NARG(ptr, "user_mask"),        S_NARG(dec, "sizemask")),
#endif
    SYSCALL(timerfd_create,          2,  S_RET(rdec),    S_NARG(dec, "clockid"),        S_NARG(arg, "flags")),
#ifdef SYS_eventfd
    SYSCALL(eventfd,                 1,  S_RET(rdec),    S_NARG(arg, "count")),
#endif
    SYSCALL(fallocate,               4,  S_RET(rdec),    S_UARG(fd),                    S_NARG(octal_mode, "mode"),      S_UARG(offset),                   S_NARG(offset, "length")),
    SYSCALL(timerfd_settime,         4,  S_RET(rdec),    S_NARG(fd, "ufd"),             S_NARG(arg, "flags"),            S_NARG(ptr, "utmr"),              S_NARG(ptr, "otmr")),
    SYSCALL(timerfd_gettime,         2,  S_RET(rdec),    S_NARG(fd, "ufd"),             S_NARG(ptr, "otmr")),
    SYSCALL(accept4,                 4,  S_RET(rdec),    S_NARG(fd, "sockfd"),          S_NARG(ptr, "addr"),             S_NARG(ptr, "addrlen"),           S_NARG(arg, "flags")),
    SYSCALL(signalfd4,               4,  S_RET(rdec),    S_NARG(fd, "ufd"),             S_NARG(ptr, "user_mask"),        S_NARG(dec, "sizemask"),          S_NARG(arg, "flags")),
    SYSCALL(eventfd2,                2,  S_RET(rdec),    S_NARG(arg, "count"),          S_NARG(arg, "flags")),
    SYSCALL(epoll_create1,           1,  S_RET(rdec),    S_NARG(arg, "flags")),
    SYSCALL(dup3,                    3,  S_RET(rdec),    S_NARG(fd, "oldfd"),           S_NARG(fd, "newfd"),             S_NARG(arg, "flags")),
    SYSCALL(pipe2,                   2,  S_RET(rdec),    S_NARG(ptr, "fildes"),         S_NARG(arg, "flags")),
    SYSCALL(inotify_init1,           1,  S_RET(rdec),    S_NARG(arg, "flags")),
    SYSCALL(preadv,                  5,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "iov"),              S_NARG(dec, "iovcnt"),            S_NARG(arg, "pos_l"),        S_NARG(arg, "pos_h")),
    SYSCALL(pwritev,                 5,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "iov"),              S_NARG(dec, "iovcnt"),            S_NARG(arg, "pos_l"),        S_NARG(arg, "pos_h")),
    SYSCALL(rt_tgsigqueueinfo,       4,  S_RET(rdec),    S_NARG(arg, "tgid"),           S_NARG(arg, "pid"),              S_NARG(signum, "sig"),            S_NARG(ptr, "uinfo")),
    SYSCALL(perf_event_open,         5,  S_RET(rdec),    S_NARG(ptr, "attr_uptr"),      S_NARG(dec, "pid"),              S_NARG(dec, "cpu"),               S_NARG(fd, "group_fd"),      S_NARG(arg, "flags")),
    SYSCALL(recvmmsg,                5,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "msg"),              S_NARG(dec, "vlen"),              S_NARG(arg, "flags"),        S_NARG(ptr, "timeout")),
    SYSCALL(fanotify_init,           2,  S_RET(rdec),    S_NARG(arg, "flags"),          S_NARG(arg, "event_f_flags")),
    SYSCALL(fanotify_mark,           5,  S_RET(rdec),    S_NARG(fd, "fanotify_fd"),     S_NARG(arg, "flags"),            S_NARG(arg, "mask"),              S_UARG(fd),                  S_NARG(cstr, "pathname")),
    SYSCALL(prlimit64,               4,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(arg, "resource"),         S_NARG(ptr, "new_rlim"),          S_NARG(ptr, "old_rlim")),
    SYSCALL(name_to_handle_at,       5,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(ptr, "handle"),            S_NARG(ptr, "mnt_id"),       S_NARG(arg, "flag")),
    SYSCALL(open_by_handle_at,       3,  S_RET(rdec),    S_NARG(fd, "mountdirfd"),      S_NARG(ptr, "handle"),           S_NARG(arg, "flags")),
    SYSCALL(clock_adjtime,           2,  S_RET(rdec),    S_NARG(arg, "which_clock"),    S_NARG(ptr, "tx")),
    SYSCALL(syncfs,                  2,  S_RET(rdec),    S_UARG(fd)),
    SYSCALL(sendmmsg,                4,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "msg"),              S_NARG(dec, "vlen"),              S_NARG(arg, "flags")),
    SYSCALL(setns,                   2,  S_RET(rdec),    S_UARG(fd),                    S_NARG(arg, "nstype")),
    SYSCALL(getcpu,                  3,  S_RET(rdec),    S_NARG(ptr, "cpu"),            S_NARG(ptr, "node"),             S_NARG(ptr, "cache")),
    SYSCALL(process_vm_readv,        6,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "local_iov"),        S_NARG(dec, "liovcnt"),           S_NARG(ptr, "remote_iov"),   S_NARG(dec, "riovcnt"),      S_NARG(arg, "flags")),
    SYSCALL(process_vm_writev,       6,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "local_iov"),        S_NARG(dec, "liovcnt"),           S_NARG(ptr, "remote_iov"),   S_NARG(dec, "riovcnt"),      S_NARG(arg, "flags")),
    SYSCALL(kcmp,                    5,  S_RET(rdec),    S_NARG(arg, "pid1"),           S_NARG(arg, "pid2"),             S_NARG(arg, "type"),              S_NARG(arg, "idx1"),         S_NARG(arg, "idx2")),
    SYSCALL(finit_module,            3,  S_RET(rdec),    S_UARG(fd),                    S_NARG(cstr, "param_values"),    S_NARG(arg, "flags")),
#ifdef SYS_sched_setattr
    SYSCALL(sched_setattr,           3,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "attr"),             S_NARG(arg, "flags")),
#endif // SYS_sched_setattr
#ifdef SYS_sched_getattr
    SYSCALL(sched_getattr,           4,  S_RET(rdec),    S_NARG(dec, "pid"),            S_NARG(ptr, "attr"),             S_NARG(dec, "size"),              S_NARG(arg, "flags")),
#endif // SYS_sched_getattr
#ifdef SYS_renameat2
    SYSCALL(renameat2,               5,  S_RET(rdec),    S_NARG(atfd, "olddfd"),        S_NARG(cstr, "oldpath"),         S_NARG(atfd, "newdfd"),           S_NARG(cstr, "newpath"),     S_NARG(arg, "flags")),
#endif // SYS_renameat2
#ifdef SYS_seccomp
    SYSCALL(seccomp,                 3,  S_RET(rdec),    S_NARG(arg, "op"),             S_NARG(arg, "flags"),            S_NARG(ptr, "uargs")),
#endif // SYS_seccomp
#ifdef SYS_getrandom
    SYSCALL(getrandom,               3,  S_RET(rdec),    S_NARG(ptr, "buf"),            S_NARG(arg, "count"),            S_NARG(arg, "flags")),
#endif // SYS_getrandom
#ifdef SYS_memfd_create
    SYSCALL(memfd_create,            2,  S_RET(rdec),    S_NARG(cstr, "pathname"),      S_NARG(arg, "flags")),
#endif // SYS_memfd_create
#ifdef SYS_kexec_file_load
    SYSCALL(kexec_file_load,         5,  S_RET(rdec),    S_NARG(fd, "kernel_fd"),       S_NARG(fd, "initrd_fd"),         S_NARG(arg, "cmdline_len"),       S_NARG(cstr, "cmdline"),     S_NARG(arg, "flags")),
#endif // SYS_kexec_file_load

#ifdef SYS_bpf
    SYSCALL(bpf,                     2,  S_RET(rdec),    S_NARG(arg, "cmd"),            S_NARG(ptr, "attr"),             S_NARG(arg, "size")),
#endif // SYS_bpf

#ifdef SYS_execveat
    SYSCALL(execveat,                5,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(ptr, "argv"),              S_NARG(ptr, "envp"),         S_NARG(arg, "flags")),
#endif // SYS_execveat

#ifdef SYS_userfaultfd
    SYSCALL(userfaultfd,             2,  S_RET(rdec),    S_NARG(arg, "flags")),
#endif // SYS_userfaultfd

#ifdef SYS_membarrier
    SYSCALL(membarrier,              2,  S_RET(rdec),    S_NARG(arg, "cmd"),            S_NARG(arg, "flags")),
#endif // SYS_membarrier

#ifdef SYS_mlock2
    SYSCALL(mlock2,                  3,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(arg, "flags")),
#endif // SYS_mlock2
#ifdef SYS_copy_file_range
    SYSCALL(copy_file_range,         6,  S_RET(rdec),    S_NARG(fd, "fd_in"),           S_NARG(ptr, "off_in"),           S_NARG(fd, "fd_out"),             S_NARG(ptr, "off_out"),      S_NARG(dec, "length"),       S_NARG(arg, "flags")),
#endif
#ifdef SYS_preadv2
    SYSCALL(preadv2,                 6,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "iov"),              S_NARG(arg, "vlen"),              S_NARG(arg, "pos_l"),        S_NARG(arg, "pos_h"),          S_NARG(arg, "flags")),
#endif // SYS_preadv2

#ifdef SYS_pwritev2
    SYSCALL(pwritev2,                6,  S_RET(rdec),    S_UARG(fd),                    S_NARG(ptr, "iov"),              S_NARG(arg, "vlen"),              S_NARG(arg, "pos_l"),        S_NARG(arg, "pos_h"),          S_NARG(arg, "flags")),
#endif // SYS_pwritev2
#ifdef SYS_pkey_mprotect
    SYSCALL(pkey_mprotect,           4,  S_RET(rdec),    S_NARG(ptr, "addr"),           S_NARG(dec, "length"),           S_NARG(mmap_prot, "prot"),        S_NARG(dec, "pkey")),
    SYSCALL(pkey_alloc,              2,  S_RET(rdec),    S_NARG(arg, "flags"),          S_NARG(arg, "init_val")),
    SYSCALL(pkey_free,               1,  S_RET(rdec),    S_NARG(dec, "pkey")),
#endif
#ifdef SYS_statx
    SYSCALL(statx,                   5,  S_RET(rdec),    S_NARG(atfd, "dfd"),           S_NARG(cstr, "pathname"),        S_NARG(arg, "flags"),             S_NARG(arg, "mask"),         S_NARG(ptr, "buffer")),
#endif // SYS_statx

#ifdef SYS_io_pgetevents
    SYSCALL(io_pgetevents,           6,  S_RET(rdec),    S_NARG(ptr, "ctx_id"),         S_NARG(dec, "min_nr"),           S_NARG(dec, "nr"),                S_NARG(ptr, "events"),       S_NARG(ptr, "timeout"),      S_NARG(ptr, "sig")),
#endif // SYS_io_pgetevents

#ifdef SYS_rseq
    SYSCALL(rseq,                    4,  S_RET(rdec),    S_NARG(ptr, "rseq"),           S_NARG(dec, "rseq_len"),         S_NARG(arg, "flags"),             S_NARG(signum, "sig"))
#endif // SYS_rseq
};

static const struct syscall_info unknown_syscall = {
    .s_name = "unknown_syscall",
    .s_nargs = MAX_SYSCALL_ARGS,
    .s_return_type = S_RET(rdec),
    .s_args = {
        S_NARG(arg, "arg0"),
        S_NARG(arg, "arg1"),
        S_NARG(arg, "arg2"),
        S_NARG(arg, "arg3"),
        S_NARG(arg, "arg4"),
        S_NARG(arg, "arg5"),
    }
};

static const struct syscall_info open_with_o_creat = {
    .s_name = "open",
    .s_nargs = 3,
    .s_return_type = S_RET(rdec),
	.s_args = {
	    S_NARG(cstr, "pathname"),
	    S_NARG(open_flags, "flags"),
	    S_NARG(octal_mode, "mode")
	}
};

static const struct syscall_info openat_with_o_creat = {
    .s_name = "openat",
    .s_nargs = 4,
    .s_return_type = S_RET(rdec),
	.s_args = {
	    S_NARG(atfd, "dfd"),
	    S_NARG(cstr, "pathname"),
	    S_NARG(open_flags, "flags"),
	    S_NARG(octal_mode, "mode")
	}
};

static bool
requires_mode_arg(int flags) {

    if((flags & O_CREAT) == O_CREAT) {
        return true;
    }

#ifdef O_TMPFILE
    if ((flags & O_TMPFILE) == O_TMPFILE) {
        return true;
    }
#endif

    return false;
}

#include <signal.h>

/**
 * get_syscall_info - Return a syscall descriptor
 *
 * This function returns a pointer to a syscall_info structure that
 * appropriately describes the system call identified by 'syscall_number'.
 */
const struct syscall_info*
get_syscall_info(const long syscall_number, const long* argv) {

    if(syscall_number < 0 ||
       syscall_number >= (long) ARRAY_SIZE(syscall_table)) {
        return &unknown_syscall;
    }

    if(syscall_table[syscall_number].s_name == NULL) {
        return &unknown_syscall;
    }

    if(argv == NULL) {
        return &syscall_table[syscall_number];
    }
#ifdef SYS_open
    if(syscall_number == SYS_open && requires_mode_arg(argv[1])) {
        return &open_with_o_creat;
    }
#endif
    if(syscall_number == SYS_openat && requires_mode_arg(argv[2])) {
        return &openat_with_o_creat;
    }

    return &syscall_table[syscall_number];
}

struct named_syscall_entry {
    const char* s_name;
    const struct syscall_info* s_info;
};

#define SYSCALL_BY_NAME(id)            \
{                                      \
    .s_name = #id,                     \
    .s_info = &syscall_table[SYS_##id] \
}

/** Linux syscalls ordered by name */
const struct named_syscall_entry syscalls_by_name[] = {
#ifdef SYS__sysctl
    SYSCALL_BY_NAME(_sysctl),
#endif
    SYSCALL_BY_NAME(accept),
    SYSCALL_BY_NAME(accept4),
#ifdef SYS_access
    SYSCALL_BY_NAME(access),
#endif
    SYSCALL_BY_NAME(acct),
    SYSCALL_BY_NAME(add_key),
    SYSCALL_BY_NAME(adjtimex),
#ifdef SYS_afs_syscall
    SYSCALL_BY_NAME(afs_syscall),
#endif
#ifdef SYS_alarm
    SYSCALL_BY_NAME(alarm),
#endif
#ifdef SYS_arch_prctl
    SYSCALL_BY_NAME(arch_prctl),
#endif // SYS_arch_prctl
    SYSCALL_BY_NAME(bind),
#ifdef SYS_bpf
    SYSCALL_BY_NAME(bpf),
#endif // SYS_bpf
    SYSCALL_BY_NAME(brk),
    SYSCALL_BY_NAME(capget),
    SYSCALL_BY_NAME(capset),
    SYSCALL_BY_NAME(chdir),
#ifdef SYS_chmod
    SYSCALL_BY_NAME(chmod),
#endif
#ifdef SYS_chown
    SYSCALL_BY_NAME(chown),
#endif
    SYSCALL_BY_NAME(chroot),
    SYSCALL_BY_NAME(clock_adjtime),
    SYSCALL_BY_NAME(clock_getres),
    SYSCALL_BY_NAME(clock_gettime),
    SYSCALL_BY_NAME(clock_nanosleep),
    SYSCALL_BY_NAME(clock_settime),
    SYSCALL_BY_NAME(clone),
    SYSCALL_BY_NAME(close),
    SYSCALL_BY_NAME(connect),
#ifdef SYS_copy_file_range
    SYSCALL_BY_NAME(copy_file_range),
#endif // SYS_copy_file_range
#ifdef SYS_creat
    SYSCALL_BY_NAME(creat),
#endif
#ifdef SYS_create_module
    SYSCALL_BY_NAME(create_module),
#endif
    SYSCALL_BY_NAME(delete_module),
    SYSCALL_BY_NAME(dup),
#ifdef SYS_dup2
    SYSCALL_BY_NAME(dup2),
#endif
    SYSCALL_BY_NAME(dup3),
#ifdef SYS_epoll_create
    SYSCALL_BY_NAME(epoll_create),
#endif
    SYSCALL_BY_NAME(epoll_create1),
    SYSCALL_BY_NAME(epoll_ctl),
#ifdef SYS_epoll_ctl_old
    SYSCALL_BY_NAME(epoll_ctl_old),
#endif // SYS_epoll_ctl_old
    SYSCALL_BY_NAME(epoll_pwait),
#ifdef SYS_epoll_wait
    SYSCALL_BY_NAME(epoll_wait),
#endif
#ifdef SYS_epoll_wait_old
    SYSCALL_BY_NAME(epoll_wait_old),
#endif // SYS_epoll_wait_old
#ifdef SYS_eventfd
    SYSCALL_BY_NAME(eventfd),
#endif
    SYSCALL_BY_NAME(eventfd2),
    SYSCALL_BY_NAME(execve),
#ifdef SYS_execveat
    SYSCALL_BY_NAME(execveat),
#endif // SYS_execveat
    SYSCALL_BY_NAME(exit),
    SYSCALL_BY_NAME(exit_group),
    SYSCALL_BY_NAME(faccessat),
#ifdef SYS_faccessat2
    SYSCALL_BY_NAME(faccessat2),
#endif
    SYSCALL_BY_NAME(fadvise64),
    SYSCALL_BY_NAME(fallocate),
    SYSCALL_BY_NAME(fanotify_init),
    SYSCALL_BY_NAME(fanotify_mark),
    SYSCALL_BY_NAME(fchdir),
    SYSCALL_BY_NAME(fchmod),
    SYSCALL_BY_NAME(fchmodat),
    SYSCALL_BY_NAME(fchown),
    SYSCALL_BY_NAME(fchownat),
    SYSCALL_BY_NAME(fcntl),
    SYSCALL_BY_NAME(fdatasync),
    SYSCALL_BY_NAME(fgetxattr),
    SYSCALL_BY_NAME(finit_module),
    SYSCALL_BY_NAME(flistxattr),
    SYSCALL_BY_NAME(flock),
#ifdef SYS_fork
    SYSCALL_BY_NAME(fork),
#endif
    SYSCALL_BY_NAME(fremovexattr),
    SYSCALL_BY_NAME(fsetxattr),
    SYSCALL_BY_NAME(fstat),
    SYSCALL_BY_NAME(fstatfs),
    SYSCALL_BY_NAME(fsync),
    SYSCALL_BY_NAME(ftruncate),
    SYSCALL_BY_NAME(futex),
#ifdef SYS_futimesat
    SYSCALL_BY_NAME(futimesat),
#endif
#ifdef SYS_get_kernel_syms
    SYSCALL_BY_NAME(get_kernel_syms),
#endif
    SYSCALL_BY_NAME(get_mempolicy),
    SYSCALL_BY_NAME(get_robust_list),
#ifdef SYS_get_thread_area
    SYSCALL_BY_NAME(get_thread_area),
#endif // SYS_get_thread_area
    SYSCALL_BY_NAME(getcpu),
    SYSCALL_BY_NAME(getcwd),
#ifdef SYS_getdents
    SYSCALL_BY_NAME(getdents),
#endif
    SYSCALL_BY_NAME(getdents64),
    SYSCALL_BY_NAME(getegid),
    SYSCALL_BY_NAME(geteuid),
    SYSCALL_BY_NAME(getgid),
    SYSCALL_BY_NAME(getgroups),
    SYSCALL_BY_NAME(getitimer),
    SYSCALL_BY_NAME(getpeername),
    SYSCALL_BY_NAME(getpgid),
#ifdef SYS_getpgrp
    SYSCALL_BY_NAME(getpgrp),
#endif
    SYSCALL_BY_NAME(getpid),
#ifdef SYS_getpmsg
    SYSCALL_BY_NAME(getpmsg),
#endif
    SYSCALL_BY_NAME(getppid),
    SYSCALL_BY_NAME(getpriority),
    SYSCALL_BY_NAME(getrandom),
    SYSCALL_BY_NAME(getresgid),
    SYSCALL_BY_NAME(getresuid),
    SYSCALL_BY_NAME(getrlimit),
    SYSCALL_BY_NAME(getrusage),
    SYSCALL_BY_NAME(getsid),
    SYSCALL_BY_NAME(getsockname),
    SYSCALL_BY_NAME(getsockopt),
    SYSCALL_BY_NAME(gettid),
    SYSCALL_BY_NAME(gettimeofday),
    SYSCALL_BY_NAME(getuid),
    SYSCALL_BY_NAME(getxattr),
    SYSCALL_BY_NAME(init_module),
    SYSCALL_BY_NAME(inotify_add_watch),
#ifdef SYS_inotify_init
    SYSCALL_BY_NAME(inotify_init),
#endif
    SYSCALL_BY_NAME(inotify_init1),
    SYSCALL_BY_NAME(inotify_rm_watch),
    SYSCALL_BY_NAME(io_cancel),
    SYSCALL_BY_NAME(io_destroy),
    SYSCALL_BY_NAME(io_getevents),
#ifdef SYS_io_pgetevents
    SYSCALL_BY_NAME(io_pgetevents),
#endif // SYS_io_pgetevents
    SYSCALL_BY_NAME(io_setup),
    SYSCALL_BY_NAME(io_submit),
    SYSCALL_BY_NAME(ioctl),
#ifdef SYS_ioperm
    SYSCALL_BY_NAME(ioperm),
#endif
#ifdef SYS_iopl
    SYSCALL_BY_NAME(iopl),
#endif
    SYSCALL_BY_NAME(ioprio_get),
    SYSCALL_BY_NAME(ioprio_set),
    SYSCALL_BY_NAME(kcmp),
#ifdef SYS_kexec_file_load
    SYSCALL_BY_NAME(kexec_file_load),
#endif
    SYSCALL_BY_NAME(kexec_load),
    SYSCALL_BY_NAME(keyctl),
    SYSCALL_BY_NAME(kill),
#ifdef SYS_lchown
    SYSCALL_BY_NAME(lchown),
#endif
    SYSCALL_BY_NAME(lgetxattr),
#ifdef SYS_link
    SYSCALL_BY_NAME(link),
#endif
    SYSCALL_BY_NAME(linkat),
    SYSCALL_BY_NAME(listen),
    SYSCALL_BY_NAME(listxattr),
    SYSCALL_BY_NAME(llistxattr),
    SYSCALL_BY_NAME(lookup_dcookie),
    SYSCALL_BY_NAME(lremovexattr),
    SYSCALL_BY_NAME(lseek),
    SYSCALL_BY_NAME(lsetxattr),
#ifdef SYS_lstat
    SYSCALL_BY_NAME(lstat),
#endif
    SYSCALL_BY_NAME(madvise),
    SYSCALL_BY_NAME(mbind),
#ifdef SYS_membarrier
    SYSCALL_BY_NAME(membarrier),
#endif // SYS_membarrier
    SYSCALL_BY_NAME(memfd_create),
    SYSCALL_BY_NAME(migrate_pages),
    SYSCALL_BY_NAME(mincore),
#ifdef SYS_mkdir
    SYSCALL_BY_NAME(mkdir),
#endif
    SYSCALL_BY_NAME(mkdirat),
#ifdef SYS_mknod
    SYSCALL_BY_NAME(mknod),
#endif
    SYSCALL_BY_NAME(mknodat),
    SYSCALL_BY_NAME(mlock),
#ifdef SYS_mlock2
    SYSCALL_BY_NAME(mlock2),
#endif // SYS_mlock2
    SYSCALL_BY_NAME(mlockall),
    SYSCALL_BY_NAME(mmap),
#ifdef SYS_modify_ldt
    SYSCALL_BY_NAME(modify_ldt),
#endif
    SYSCALL_BY_NAME(mount),
    SYSCALL_BY_NAME(move_pages),
    SYSCALL_BY_NAME(mprotect),
    SYSCALL_BY_NAME(mq_getsetattr),
    SYSCALL_BY_NAME(mq_notify),
    SYSCALL_BY_NAME(mq_open),
    SYSCALL_BY_NAME(mq_timedreceive),
    SYSCALL_BY_NAME(mq_timedsend),
    SYSCALL_BY_NAME(mq_unlink),
    SYSCALL_BY_NAME(mremap),
#ifdef SYS_msgctl
    SYSCALL_BY_NAME(msgctl),
#endif // SYS_msgctl
#ifdef SYS_msgget
    SYSCALL_BY_NAME(msgget),
#endif // SYS_msgget
#ifdef SYS_msgrcv
    SYSCALL_BY_NAME(msgrcv),
#endif // SYS_msgrcv
#ifdef SYS_msgsnd
    SYSCALL_BY_NAME(msgsnd),
#endif // SYS_msgsnd
    SYSCALL_BY_NAME(msync),
    SYSCALL_BY_NAME(munlock),
    SYSCALL_BY_NAME(munlockall),
    SYSCALL_BY_NAME(munmap),
    SYSCALL_BY_NAME(name_to_handle_at),
    SYSCALL_BY_NAME(nanosleep),
    SYSCALL_BY_NAME(newfstatat),
    SYSCALL_BY_NAME(nfsservctl),
#ifdef SYS_open
    SYSCALL_BY_NAME(open),
#endif
    SYSCALL_BY_NAME(open_by_handle_at),
    SYSCALL_BY_NAME(openat),
#ifdef SYS_pause
    SYSCALL_BY_NAME(pause),
#endif
    SYSCALL_BY_NAME(perf_event_open),
    SYSCALL_BY_NAME(personality),
#ifdef SYS_pipe
    SYSCALL_BY_NAME(pipe),
#endif
    SYSCALL_BY_NAME(pipe2),
    SYSCALL_BY_NAME(pivot_root),
#ifdef SYS_poll
    SYSCALL_BY_NAME(poll),
#endif
    SYSCALL_BY_NAME(ppoll),
    SYSCALL_BY_NAME(prctl),
    SYSCALL_BY_NAME(pread64),
    SYSCALL_BY_NAME(preadv),
#ifdef SYS_preadv2
    SYSCALL_BY_NAME(preadv2),
#endif // SYS_preadv2
#ifdef SYS_pkey_mprotect
    SYSCALL_BY_NAME(pkey_mprotect),
#endif // SYS_pkey_mprotect
#ifdef SYS_pkey_alloc
    SYSCALL_BY_NAME(pkey_alloc),
#endif // SYS_pkey_alloc
#ifdef SYS_pkey_free
    SYSCALL_BY_NAME(pkey_free),
#endif // SYS_pkey_free
    SYSCALL_BY_NAME(prlimit64),
    SYSCALL_BY_NAME(process_vm_readv),
    SYSCALL_BY_NAME(process_vm_writev),
    SYSCALL_BY_NAME(pselect6),
    SYSCALL_BY_NAME(ptrace),
#ifdef SYS_putpmsg
    SYSCALL_BY_NAME(putpmsg),
#endif
    SYSCALL_BY_NAME(pwrite64),
    SYSCALL_BY_NAME(pwritev),
#ifdef SYS_pwritev2
    SYSCALL_BY_NAME(pwritev2),
#endif // SYS_pwritev2
#ifdef SYS_query_module
    SYSCALL_BY_NAME(query_module),
#endif
    SYSCALL_BY_NAME(quotactl),
    SYSCALL_BY_NAME(read),
    SYSCALL_BY_NAME(readahead),
#ifdef SYS_readlink
    SYSCALL_BY_NAME(readlink),
#endif
    SYSCALL_BY_NAME(readlinkat),
    SYSCALL_BY_NAME(readv),
    SYSCALL_BY_NAME(reboot),
    SYSCALL_BY_NAME(recvfrom),
    SYSCALL_BY_NAME(recvmmsg),
    SYSCALL_BY_NAME(recvmsg),
    SYSCALL_BY_NAME(remap_file_pages),
    SYSCALL_BY_NAME(removexattr),
#ifdef SYS_rename
    SYSCALL_BY_NAME(rename),
#endif
    SYSCALL_BY_NAME(renameat),
    SYSCALL_BY_NAME(renameat2),
    SYSCALL_BY_NAME(request_key),
    SYSCALL_BY_NAME(restart_syscall),
#ifdef SYS_rmdir
    SYSCALL_BY_NAME(rmdir),
#endif
#ifdef SYS_rseq
    SYSCALL_BY_NAME(rseq),
#endif // SYS_rseq
    SYSCALL_BY_NAME(rt_sigaction),
    SYSCALL_BY_NAME(rt_sigpending),
    SYSCALL_BY_NAME(rt_sigprocmask),
    SYSCALL_BY_NAME(rt_sigqueueinfo),
    SYSCALL_BY_NAME(rt_sigreturn),
    SYSCALL_BY_NAME(rt_sigsuspend),
    SYSCALL_BY_NAME(rt_sigtimedwait),
    SYSCALL_BY_NAME(rt_tgsigqueueinfo),
    SYSCALL_BY_NAME(sched_get_priority_max),
    SYSCALL_BY_NAME(sched_get_priority_min),
    SYSCALL_BY_NAME(sched_getaffinity),
    SYSCALL_BY_NAME(sched_getattr),
    SYSCALL_BY_NAME(sched_getparam),
    SYSCALL_BY_NAME(sched_getscheduler),
    SYSCALL_BY_NAME(sched_rr_get_interval),
    SYSCALL_BY_NAME(sched_setaffinity),
    SYSCALL_BY_NAME(sched_setattr),
    SYSCALL_BY_NAME(sched_setparam),
    SYSCALL_BY_NAME(sched_setscheduler),
    SYSCALL_BY_NAME(sched_yield),
    SYSCALL_BY_NAME(seccomp),
#ifdef SYS_security
    SYSCALL_BY_NAME(security),
#endif // SYS_security
#ifdef SYS_select
    SYSCALL_BY_NAME(select),
#endif
#ifdef SYS_semctl
    SYSCALL_BY_NAME(semctl),
#endif // SYS_semctl
#ifdef SYS_semget
    SYSCALL_BY_NAME(semget),
#endif // SYS_semget
#ifdef SYS_semop
    SYSCALL_BY_NAME(semop),
#endif // SYS_semop
#ifdef SYS_semtimedop
    SYSCALL_BY_NAME(semtimedop),
#endif // SYS_semtimedop
    SYSCALL_BY_NAME(sendfile),
    SYSCALL_BY_NAME(sendmmsg),
    SYSCALL_BY_NAME(sendmsg),
    SYSCALL_BY_NAME(sendto),
    SYSCALL_BY_NAME(set_mempolicy),
    SYSCALL_BY_NAME(set_robust_list),
#ifdef SYS_set_thread_area
    SYSCALL_BY_NAME(set_thread_area),
#endif // SYS_set_thread_area
    SYSCALL_BY_NAME(set_tid_address),
    SYSCALL_BY_NAME(setdomainname),
    SYSCALL_BY_NAME(setfsgid),
    SYSCALL_BY_NAME(setfsuid),
    SYSCALL_BY_NAME(setgid),
    SYSCALL_BY_NAME(setgroups),
    SYSCALL_BY_NAME(sethostname),
    SYSCALL_BY_NAME(setitimer),
    SYSCALL_BY_NAME(setns),
    SYSCALL_BY_NAME(setpgid),
    SYSCALL_BY_NAME(setpriority),
    SYSCALL_BY_NAME(setregid),
    SYSCALL_BY_NAME(setresgid),
    SYSCALL_BY_NAME(setresuid),
    SYSCALL_BY_NAME(setreuid),
    SYSCALL_BY_NAME(setrlimit),
    SYSCALL_BY_NAME(setsid),
    SYSCALL_BY_NAME(setsockopt),
    SYSCALL_BY_NAME(settimeofday),
    SYSCALL_BY_NAME(setuid),
    SYSCALL_BY_NAME(setxattr),
#ifdef SYS_shmat
    SYSCALL_BY_NAME(shmat),
#endif // SYS_shmat
#ifdef SYS_shmctl
    SYSCALL_BY_NAME(shmctl),
#endif // SYS_shmctl
#ifdef SYS_shmdt
    SYSCALL_BY_NAME(shmdt),
#endif // SYS_shmdt
#ifdef SYS_shmget
    SYSCALL_BY_NAME(shmget),
#endif // SYS_shmget
    SYSCALL_BY_NAME(shutdown),
    SYSCALL_BY_NAME(sigaltstack),
#ifdef SYS_signalfd
    SYSCALL_BY_NAME(signalfd),
#endif
    SYSCALL_BY_NAME(signalfd4),
    SYSCALL_BY_NAME(socket),
    SYSCALL_BY_NAME(socketpair),
    SYSCALL_BY_NAME(splice),
#ifdef SYS_stat
    SYSCALL_BY_NAME(stat),
#endif
    SYSCALL_BY_NAME(statfs),
#ifdef SYS_statx
    SYSCALL_BY_NAME(statx),
#endif // SYS_statx
    SYSCALL_BY_NAME(swapoff),
    SYSCALL_BY_NAME(swapon),
#ifdef SYS_symlink
    SYSCALL_BY_NAME(symlink),
#endif
    SYSCALL_BY_NAME(symlinkat),
    SYSCALL_BY_NAME(sync),
#ifdef SYS_sync_file_range
    SYSCALL_BY_NAME(sync_file_range),
#endif // SYS_sync_file_range
    SYSCALL_BY_NAME(syncfs),
#ifdef SYS_sysfs
    SYSCALL_BY_NAME(sysfs),
#endif
    SYSCALL_BY_NAME(sysinfo),
    SYSCALL_BY_NAME(syslog),
    SYSCALL_BY_NAME(tee),
    SYSCALL_BY_NAME(tgkill),
#ifdef SYS_time
    SYSCALL_BY_NAME(time),
#endif
    SYSCALL_BY_NAME(timer_create),
    SYSCALL_BY_NAME(timer_delete),
    SYSCALL_BY_NAME(timer_getoverrun),
    SYSCALL_BY_NAME(timer_gettime),
    SYSCALL_BY_NAME(timer_settime),
    SYSCALL_BY_NAME(timerfd_create),
    SYSCALL_BY_NAME(timerfd_gettime),
    SYSCALL_BY_NAME(timerfd_settime),
    SYSCALL_BY_NAME(times),
    SYSCALL_BY_NAME(tkill),
    SYSCALL_BY_NAME(truncate),
#ifdef SYS_tuxcall
    SYSCALL_BY_NAME(tuxcall),
#endif
    SYSCALL_BY_NAME(umask),
    SYSCALL_BY_NAME(umount2),
    SYSCALL_BY_NAME(uname),
#ifdef SYS_unlink
    SYSCALL_BY_NAME(unlink),
#endif
    SYSCALL_BY_NAME(unlinkat),
    SYSCALL_BY_NAME(unshare),
#ifdef SYS_uselib
    SYSCALL_BY_NAME(uselib),
#endif
    SYSCALL_BY_NAME(userfaultfd),
#ifdef SYS_ustat
    SYSCALL_BY_NAME(ustat),
#endif
#ifdef SYS_utime
    SYSCALL_BY_NAME(utime),
#endif
    SYSCALL_BY_NAME(utimensat),
#ifdef SYS_utimes
    SYSCALL_BY_NAME(utimes),
#endif
#ifdef SYS_vfork
    SYSCALL_BY_NAME(vfork),
#endif
    SYSCALL_BY_NAME(vhangup),
    SYSCALL_BY_NAME(vmsplice),
#ifdef SYS_vserver
    SYSCALL_BY_NAME(vserver),
#endif // SYS_vserver
    SYSCALL_BY_NAME(wait4),
    SYSCALL_BY_NAME(waitid),
    SYSCALL_BY_NAME(write),
    SYSCALL_BY_NAME(writev),
};

static int
compare_named_entries(const void* k, const void* e) {
    const char* name = (const char*) k;
    struct named_syscall_entry* entry = (struct named_syscall_entry*) e;
    return strcmp(name, entry->s_name);
}

const struct syscall_info*
get_syscall_info_by_name(const char* syscall_name) {

    struct named_syscall_entry* res = bsearch(
            syscall_name, &syscalls_by_name[0], ARRAY_SIZE(syscalls_by_name),
            sizeof(struct named_syscall_entry), compare_named_entries);

    if(res == NULL) {
        return &unknown_syscall;
    }

    return res->s_info;
}

#define RETURN_TYPE(scinfo) (scinfo)->s_return_type.r_type

bool
syscall_never_returns(long syscall_number) {
    return RETURN_TYPE(get_syscall_info(syscall_number, NULL)) == rnone;
}


#undef SYSCALL
#undef S_NOARGS
#undef S_UARG
#undef S_NARG
#undef S_RET
#undef SYSCALL_BY_NAME
#undef ARRAY_SIZE

// clang-format on
