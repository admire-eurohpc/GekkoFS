# GekkoFS

[![License: GPL3](https://img.shields.io/badge/License-GPL3-blue.svg)](https://opensource.org/licenses/GPL-3.0)
[![pipeline status](https://storage.bsc.es/gitlab/hpc/gekkofs/badges/master/pipeline.svg)](https://storage.bsc.es/gitlab/hpc/gekkofs/commits/master)
[![coverage report](https://storage.bsc.es/gitlab/hpc/gekkofs/badges/master/coverage.svg)](https://storage.bsc.es/gitlab/hpc/gekkofs/-/commits/master)

GekkoFS is a file system capable of aggregating the local I/O capacity and performance of each compute node
in a HPC cluster to produce a high-performance storage space that can be accessed in a distributed manner.
This storage space allows HPC applications and simulations to run in isolation from each other with regards
to I/O, which reduces interferences and improves performance.

# Dependencies

- \>gcc-8 (including g++) for C++11 support
- General build tools: Git, Curl, CMake >3.6 (>3.11 for GekkoFS testing), Autoconf, Automake
- Miscellaneous: Libtool, Libconfig

### Debian/Ubuntu

GekkoFS base dependencies: `apt install git curl cmake autoconf automake libtool libconfig-dev`

GekkoFS testing support: `apt install python3-dev python3 python3-venv`

With testing

### CentOS/Red Hat

GekkoFS base dependencies: `yum install gcc-c++ git curl cmake autoconf automake libtool libconfig`

GekkoFS testing support: `python38-devel` (**>Python-3.6 required**)

# Step-by-step installation

1. Make sure the above listed dependencies are available on your machine
2. Clone GekkoFS: `git clone --recurse-submodules https://storage.bsc.es/gitlab/hpc/gekkofs.git`
    - (Optional) (Optional) If you checked out the sources using `git` without the `--recursive` option, you need to
      execute the following command from the root of the source directory: `git submodule update --init`
3. Set up the necessary environment variables where the compiled direct GekkoFS dependencies will be installed at (we
   assume the path `/home/foo/gekkofs_deps/install` in the following)
    - `export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/foo/gekkofs_deps/install/lib:/home/foo/gekkofs_deps/install/lib64`
4. Download and compile the direct dependencies, e.g.,
    - Download example: `gekkofs/scripts/dl_dep.sh /home/foo/gekkofs_deps/git`
    - Compilation example: `gekkofs/scripts/compile_dep.sh /home/foo/gekkofs_deps/git /home/foo/gekkofs_deps/install`
    - Consult `-h` for additional arguments for each script
5. Compile GekkoFS and run optional tests
    - Create build directory: `mkdir gekkofs/build && cd gekkofs/build`
    - Configure GekkoFS: `cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/home/foo/gekkofs_deps/install ..`
        - add `-DCMAKE_INSTALL_PREFIX=<install_path>` where the GekkoFS client library and server executable should be
          available
        - add `-DGKFS_BUILD_TESTS=ON` if tests should be build
    - Build and install GekkoFS: `make -j8 install`
    - Run tests: `make test`

GekkoFS is now available at:

- GekkoFS daemon (server): `<install_path>/bin/gkfs_daemon`
- GekkoFS client interception library: `<install_path>/lib64/libgkfs_intercept.so`

## Use Spack to install GekkoFS (alternative)

The Spack tool can be used to easily install GekkoFS and its dependencies. Refer to the
following [README](scripts/spack/README.md) for details.

# Run GekkoFS

## General

On each node a daemon (`gkfs_daemon` binary) has to be started. Other tools can be used to execute
the binary on many nodes, e.g., `srun`, `mpiexec/mpirun`, `pdsh`, or `pssh`.

You need to decide what Mercury NA plugin you want to use for network communication. `ofi+sockets` is the default.
The `-P` argument is used for setting another RPC protocol. See below.

- `ofi+sockets` for using the libfabric plugin with TCP (stable)
- `ofi+tcp` for using the libfabric plugin with TCP (slower than sockets)
- `ofi+verbs` for using the libfabric plugin with Infiniband verbs (reasonably stable) and requires
  the [rdma-core (formerly libibverbs)](https://github.com/linux-rdma/rdma-core) library
- `ofi+psm2` for using the libfabric plugin with Intel Omni-Path (unstable) and requires
  the [opa-psm2](https://github.com/cornelisnetworks/opa-psm2>) library

## The GekkoFS hostsfile

Each GekkoFS daemon needs to register itself in a shared file (*hostsfile*) which needs to be accessible to _all_
GekkoFS clients and daemons.
Therefore, the hostsfile describes a file system and which node is part of that specific GekkoFS file system instance.
In a typical cluster environment this hostsfile should be placed within a POSIX-compliant parallel file system, such as
GPFS or Lustre.

*Note: NFS is not strongly consistent and cannot be used for the hosts file!*

## GekkoFS daemon start and shut down

tl;dr example: `<install_path>/bin/gkfs_daemon -r <fs_data_path> -m <pseudo_gkfs_mount_dir_path> -H <hostsfile_path>`

Run the GekkoFS daemon on each node specifying its locally used directory where the file system data and metadata is
stored (`-r/--rootdir <fs_data_path>`), e.g., the node-local SSD;

2. the pseudo mount directory used by clients to access GekkoFS (`-m/--mountdir <pseudo_gkfs_mount_dir_path>`); and
3. the hostsfile path (`-H/--hostsfile <hostfile_path>`).

Further options are available:

```bash
Allowed options
Usage: src/daemon/gkfs_daemon [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -m,--mountdir TEXT REQUIRED Virtual mounting directory where GekkoFS is available.
  -r,--rootdir TEXT REQUIRED  Local data directory where GekkoFS data for this daemon is stored.
  -s,--rootdir-suffix TEXT    Creates an additional directory within the rootdir, allowing multiple daemons on one node.
  -i,--metadir TEXT           Metadata directory where GekkoFS RocksDB data directory is located. If not set, rootdir is used.
  -l,--listen TEXT            Address or interface to bind the daemon to. Default: local hostname.
                              When used with ofi+verbs the FI_VERBS_IFACE environment variable is set accordingly which associates the verbs device with the network interface. In case FI_VERBS_IFACE is already defined, the argument is ignored. Default 'ib'.
  -H,--hosts-file TEXT        Shared file used by deamons to register their endpoints. (default './gkfs_hosts.txt')
  -P,--rpc-protocol TEXT      Used RPC protocol for inter-node communication.
                              Available: {ofi+sockets, ofi+verbs, ofi+psm2} for TCP, Infiniband, and Omni-Path, respectively. (Default ofi+sockets)
                              Libfabric must have enabled support verbs or psm2.
  --auto-sm                   Enables intra-node communication (IPCs) via the `na+sm` (shared memory) protocol, instead of using the RPC protocol. (Default off)
  --clean-rootdir             Cleans Rootdir >before< launching the deamon
  -c,--clean-rootdir-finish   Cleans Rootdir >after< the deamon finishes
  -d,--dbbackend TEXT         Metadata database backend to use. Available: {rocksdb, parallaxdb}
                              RocksDB is default if not set. Parallax support is experimental.
                              Note, parallaxdb creates a file called rocksdbx with 8GB created in metadir.
  --parallaxsize TEXT         parallaxdb - metadata file size in GB (default 8GB), used only with new files
  --enable-collection         Enables collection of general statistics. Output requires either the --output-stats or --enable-prometheus argument.
  --enable-chunkstats         Enables collection of data chunk statistics in I/O operations.Output requires either the --output-stats or --enable-prometheus argument.
  --output-stats TEXT         Creates a thread that outputs the server stats each 10s to the specified file.
  --enable-prometheus         Enables prometheus output and a corresponding thread.
  --prometheus-gateway TEXT   Defines the prometheus gateway <ip:port> (Default 127.0.0.1:9091).
  --version                   Print version and exit.
```

It is possible to run multiple independent GekkoFS instances on the same node. Note, that when these GekkoFS instances
are part of the same file system, use the same `rootdir` with different `rootdir-suffixe`s.

Shut it down by gracefully killing the process (SIGTERM).

## Use the GekkoFS client library

tl;dr example:

```bash
export LIBGKFS_ HOSTS_FILE=<hostfile_path>
LD_PRELOAD=<install_path>/lib64/libgkfs_intercept.so cp ~/some_input_data <pseudo_gkfs_mount_dir_path>/some_input_data
LD_PRELOAD=<install_path>/lib64/libgkfs_intercept.so md5sum ~/some_input_data <pseudo_gkfs_mount_dir_path>/some_input_data
```

Clients read the hostsfile to determine which daemons are part of the GekkoFS instance. Because the client is an
interposition library that is loaded within the context of the application, this information is passed via the
environment variable `LIBGKFS_HOSTS_FILE` pointing to the hostsfile path. The client library itself is loaded for each
application process via the `LD_PRELOAD` environment variable intercepting file system related calls. If they are
within (or hierarchically under) the GekkoFS mount directory they are processed in the library, otherwise they are
passed to the kernel.

Note, if `LD_PRELOAD` is not pointing to the library and, hence the client is not loaded, the mounting directory appears
to be empty.

For MPI application, the `LD_PRELOAD` variable can be passed with the `-x` argument for `mpirun/mpiexec`.

## Run GekkoFS daemons on multiple nodes (beta version!)

The `scripts/run/gkfs` script can be used to simplify starting the GekkoFS daemon on one or multiple nodes. To start
GekkoFS on multiple nodes, a Slurm environment that can execute `srun` is required. Users can further
modify `scripts/run/gkfs.conf` to mold default configurations to their environment.

The following options are available for `scripts/run/gkfs`:

```bash
usage: gkfs [-h/--help] [-r/--rootdir <path>] [-m/--mountdir <path>] [-a/--args <daemon_args>] [-f/--foreground <false>]
        [--srun <false>] [-n/--numnodes <jobsize>] [--cpuspertask <64>] [--numactl <false>] [-v/--verbose <false>]
        {start,stop}


    This script simplifies the starting and stopping GekkoFS daemons. If daemons are started on multiple nodes,
    a Slurm environment is required. The script looks for the 'gkfs.conf' file in the same directory where
    additional permanent configurations can be set.

    positional arguments:
            command                 Command to execute: 'start' and 'stop'

    optional arguments:
            -h, --help              Shows this help message and exits
            -r, --rootdir <path>    Providing the rootdir path for GekkoFS daemons.
            -m, --mountdir <path>   Providing the mountdir path for GekkoFS daemons.
            -a, --args <daemon_arguments>
                                    Add various additional daemon arguments, e.g., "-l ib0 -P ofi+psm2".
            -f, --foreground        Starts the script in the foreground. Daemons are stopped by pressing 'q'.
            --srun                  Use srun to start daemons on multiple nodes.
            -n, --numnodes <n>      GekkoFS daemons are started on n nodes.
                                    Nodelist is extracted from Slurm via the SLURM_JOB_ID env variable.
            --cpuspertask <#cores>  Set the number of cores the daemons can use. Must use '--srun'.
            --numactl               Use numactl for the daemon. Modify gkfs.conf for further numactl configurations.
            -c, --config            Path to configuration file. By defaults looks for a 'gkfs.conf' in this directory.
            -v, --verbose           Increase verbosity
```

### Logging

The following environment variables can be used to enable logging in the client library: `LIBGKFS_LOG=<module>`
and `LIBGKFS_LOG_OUTPUT=<path/to/file>` to configure the output module and set the path to the log file of the client
library. If not path is specified in `LIBGKFS_LOG_OUTPUT`, the client library will send log messages
to `/tmp/gkfs_client.log`. Use `LIBGKFS_LOG_PER_PROCESS=ON` to write separate logs per client process.
When enabled, the path specified with `LIBGKFS_LOG_OUTPUT=<path/to/dir>` is used as a directory.

The following modules are available:

- `none`: don't print any messages
- `syscalls`: Trace system calls: print the name of each system call, its
  arguments, and its return value. All system calls are printed after being
  executed save for those that may not return, such as `execve()`,
  `execve_at()`, `exit()`, and `exit_group()`. This module will only be
  available if the client library is built in `Debug` mode.
- `syscalls_at_entry`: Trace system calls: print the name of each system call
  and its arguments. All system calls are printed before being executed and
  therefore their return values are not available in the log. This module will
  only be available if the client library is built in `Debug` mode.
- `info`: Print information messages.
- `critical`: Print critical errors.
- `errors`: Print errors.
- `warnings`: Print warnings.
- `mercury`: Print Mercury messages.
- `debug`: Print debug messages. This module will only be available if the
  client library is built in `Debug` mode.
- `most`: All previous options combined except `syscalls_at_entry`. This
  module will only be available if the client library is built in `Debug`
  mode.
- `all`: All previous options combined.
- `trace_reads`: Generate log line with extra information in read operations for guided distributor
- `help`: Print a help message and exit.

When tracing sytem calls, specific syscalls can be removed from log messages by
setting the `LIBGKFS_LOG_SYSCALL_FILTER` environment variable. For instance,
setting it to `LIBGKFS_LOG_SYSCALL_FILTER=epoll_wait,epoll_create` will filter
out any log entries from the `epoll_wait()` and `epoll_create()` system calls.

Additionally, setting the `LIBGKFS_LOG_OUTPUT_TRUNC` environment variable with a value different from `0` will instruct
the logging subsystem to truncate the file used for logging, rather than append to it.

For the daemon, the `GKFS_DAEMON_LOG_PATH=<path/to/file>` environment variable can be provided to set the path to the
log file, and the log module can be selected with the `GKFS_DAEMON_LOG_LEVEL={off,critical,err,warn,info,debug,trace}`
environment variable.

# Miscellaneous

## External functions

GekkoFS allows to use external functions on your client code, via LD_PRELOAD.
Source code needs to be compiled with -fPIC. We include a pfind io500 substitution,
`examples/gfind/gfind.cpp` and a non-mpi version `examples/gfind/sfind.cpp`

## Data distributors

The data distribution can be selected at compilation time, we have 2 distributors available:

### Simple Hash (Default)

Chunks are distributed randomly to the different GekkoFS servers.

### Guided Distributor

The guided distributor allows defining a specific distribution of data on a per directory or file basis.
The distribution configurations are defined within a shared file (called `guided_config.txt` henceforth) with the
following format:
`<path> <chunk_number> <host>`

To enable the distributor, the following CMake compilation flags are required:

* `GKFS_USE_GUIDED_DISTRIBUTION` ON
* `GKFS_USE_GUIDED_DISTRIBUTION_PATH` `<path_guided_config.txt>`

To use a custom distribution, a path needs to have the prefix `#` (e.g., `#/mdt-hard 0 0`), in which all the data of all
files in that directory goes to the same place as the metadata.
Note, that a chunk/host configuration is inherited to all children files automatically even if not using the prefix.
In this example, `/mdt-hard/file1` is therefore also using the same distribution as the `/mdt-hard` directory.
If no prefix is used, the Simple Hash distributor is used.

#### Guided configuration file

Creating a guided configuration file is based on an I/O trace file of a previous execution of the application.
For this the `trace_reads` tracing module is used (see above).

The `trace_reads` module enables a `TRACE_READS` level log at the clients writing the I/O information of the client
which is used as the input for a script that creates the guided distributor setting.
Note that capturing the necessary trace records can involve performance degradation.
To capture the I/O of each client within a SLURM environment, i.e., enabling the `trace_reads` module and print its
output to a user-defined path, the following example can be used:
`srun -N 10 -n 320 --export="ALL" /bin/bash -c "export LIBGKFS_LOG=trace_reads;LIBGKFS_LOG_OUTPUT=${HOME}/test/GLOBAL.txt;LD_PRELOAD=${GKFS_PRLD} <app>"`

Then, the `examples/distributors/guided/generate.py` scrpt is used to create the guided distributor configuration file:

* `python examples/distributors/guided/generate.py ~/test/GLOBAL.txt >> guided_config.txt`

Finally, modify `guided_config.txt` to your distribution requirements.

## Metadata Backends

There are two different metadata backends in GekkoFS. The default one uses `rocksdb`, however an alternative based
on `PARALLAX` from `FORTH`
is available. To enable it use the `-DGKFS_ENABLE_PARALLAX:BOOL=ON` option, you can also disable `rocksdb`
with `-DGKFS_ENABLE_ROCKSDB:BOOL=OFF`.

Once it is enabled, `--dbbackend` option will be functional.

## Statistics

GekkoFS daemons are able to output general operations (`--enable-collection`) and data chunk
statistics (`--enable-chunkstats`) to a specified output file via `--output-stats <FILE>`. Prometheus can also be used
instead or in addition to the output file. It must be enabled at compile time via the CMake
argument `-DGKFS_ENABLE_PROMETHEUS` and the daemon argument `--enable-prometheus`. The corresponding statistics are then
pushed to the Prometheus instance.

## Advanced and experimental features

### Rename

`-DGKFS_RENAME_SUPPORT` allows the application to rename files.
This is an experimental feature, and some scenarios may not work properly.
Support for fstat in renamed files is included.

This is disabled by default.

### Replication

The user can enable the data replication feature by setting the replication environment variable:
`LIBGKFS_NUM_REPL=<num repl>`.
The number of replicas should go from `0` to the `number of servers - 1`. The replication environment variable can be
set up for each client independently.

### Client metrics via MessagePack and ZeroMQ

GekkoFS clients support capturing the I/O traces of each individual process and periodically exporting them to a given
file or ZeroMQ sink via the TCP protocol.
To use this feature, the corresponding ZeroMQ (`libzmq` and `cppzmq`) dependencies are required which can be found in
the `default_zmq` dependency profile.
In addition, GekkoFS must be compiled with client metrics enabled (disabled by default) via the CMake argument
`-DGKFS_ENABLE_CLIENT_METRICS=ON`.

Client metrics are individually enabled per GekkoFS client process via the following environment variables:

- `LIBGKFS_ENABLE_METRICS=ON` enables capturing client-side metrics.
- `LIBGKFS_METRICS_FLUSH_INTERVAL=10` sets the flush interval to 10 seconds (defaults to 5). All outstanding client
  metrics are flushed when the process ends.
- `LIBGKFS_METRICS_PATH=<path>` sets the path to flush client-metrics (defaults to `/tmp/gkfs_client_metrics`).
- `LIBGKFS_METRICS_IP_PORT=127.0.0.1:5555` enables flushing to a set ZeroMQ server. This option disables flushing to a
  file.

The ZeroMQ export can be tested via the `gkfs_clientmetrics2json` application which is built when enabling the CMake
option `-DGKFS_BUILD_TOOLS=ON`:

- Starting the ZeroMQ server: `gkfs_clientmetrics2json tcp://127.0.0.1:5555`
- `gkfs_clientmetrics2json <path>` can also be used to unpack the Messagepack export from a file.
  Examplarily output with the ZeroMQ sink enabled when running:
  `LD_PRELOAD=libgkfs_intercept.so LIBGKFS_ENABLE_METRICS=ON LIBGKFS_METRICS_IP_PORT=127.0.0.1:5555 gkfs cp testfile /tmp/gkfs_mountdir/testfile`:

```bash
~ $ gkfs_clientmetrics2json tcp://127.0.0.1:5555
Binding to: tcp://127.0.0.1:5555
Waiting for message...

Received message with size 68
Generated JSON:
[extra]avg_thruput_mib: [221.93,175.87,266.81,135.69]
end_t_micro: [8008,12396,16006,18454]
flush_t: 18564
hostname: "evie"
io_type: "w"
pid: 1259304
req_size: [524288,524288,524288,229502]
start_t_micro: [5755,9553,14132,16841]
total_bytes: 1802366
total_iops: 4
```
The user can enable the data replication feature by setting the replication environment variable:
`LIBGKFS_NUM_REPL=<num repl>`.
The number of replicas should go from 0 to the number of servers-1. 
The replication environment variable can be set up for each client, independently.

### Erasure codes
The user can enable resilience with erasure codes with `-DGKFS_ENABLE_EC`
Using `LIBGKFS_NUM_REPL=<num repl>`, the user can define the number of EC servers.
The total servers available for data will be -> total servervs - num_repl

Additionaly the user can enable the `-DGKFS_ENABLE_READ_ERRORS` to inject a  
50% of read errors into the workflow.

## Acknowledgment

This software was partially supported by the EC H2020 funded NEXTGenIO project (Project ID: 671951, www.nextgenio.eu).

This software was partially supported by the ADA-FS project under the SPPEXA project (http://www.sppexa.de/) funded by
the DFG.

This software is partially supported by the FIDIUM project funded by the DFG.

This work was partially funded by the European Union’s Horizon 2020 and the German Ministry of Education and Research (
BMBF) under the ``Adaptive multi-tier intelligent data manager for Exascale (ADMIRE)''
project (https://www.admire-eurohpc.eu/); Grant Agreement number:
956748-ADMIRE-H2020-JTI-EuroHPC-2019-1. Further, this work was partially supported by the Spanish Ministry of Economy
and Competitiveness (MINECO) under grants PID2019-107255GB, and the Generalitat de Catalunya under contract
2021-SGR-00412. This publication is part of the project ADMIRE PCI2021-121952, funded by MCIN/AEI/10.13039/501100011033.
