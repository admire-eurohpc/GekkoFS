.. _building_gekkofs:

Installing GekkoFS
******************

This section describes how to build and install GekkoFS.

.. _gkfs_base_dependencies:

Base dependencies
=================

GekkoFS relies on CMake for building and testing the daemon and client
binaries, which are written in C++17. As such, GekkoFS has the following
base dependencies for its build system:

- A compiler with C++17 support:

  - `GCC <https://gcc.gnu.org>`_ version 8.0 and newer
  - `Clang <https://clang.llvm.org/>`_ 5.0 and newer.

- `CMake <https://cmake.org>`_ version 3.6 or newer to build the main code.

  - Version 3.11 or newer to build the tests.

GekkoFS also needs :code:`curl` and :code:`git` to be installed in
your system if you plan on using the dependency management scripts provided
with the code, as well as `Python 3.6 <https://www.python.org/downloads/>`_
or later to run some of the tests.

Additionally, since some GekkoFS dependencies rely on `GNU Automake
<https://www.gnu.org/software/automake/>`_ as
their build system, it is also recommended to install the latest version of
:code:`autoconf`, :code:`automake`, and :code:`libtool`.

This set of dependencies are typically available in the system repositories.
We show below how to install them for some of the most popular Linux
distributions:

Debian/Ubuntu
-------------

.. code-block:: console

  # Base dependencies
  $ apt install git curl cmake autoconf automake libtool libconfig-dev

  # Testing support:
  $ apt install python3-dev python3 python3-venv

CentOS/Red Hat
--------------

.. code-block:: console

  # Base dependencies:
  $ yum install gcc-c++ git curl cmake autoconf automake libtool libconfig

  # Testing support:
  $ yum install python38-devel

.. _gkfs_dependencies:

Software dependencies
=====================

GekkoFS requires several software packages to be available in your system in
order to function properly. We list them here for informational purposes.

.. warning::

    Though it is possible to install all dependencies manually, GekkoFS
    provides some dependency management scripts that automate and
    greatly simplify this task. We strongly suggest that you take read through
    the :ref:`step-by-step installation<step_by_step_installation>` section
    before attempting a manual install.

- `RocksDB <https://github.com/facebook/rocksdb/>`_ version 6.2.2 or newer and its used compression library:

  - `lz4 <https://github.com/lz4/lz4>`_ version 1.8.0 or newer.


- `Margo <https://github.com/mochi-hpc/mochi-margo/releases>`_ version 0.6.3 and its dependencies:

  - `Argobots <https://github.com/pmodels/argobots/releases/tag/v1.0.1>`_ version 1.0rc1.
  - `Mercury <https://github.com/mercury-hpc/mercury/releases/tag/v2.0.0>`_ version 2.0.0.

    - `libfabric <https://github.com/ofiwg/libfabric>`_ version 1.9 or newer.


- `syscall_intercept <https://github.com/pmem/syscall_intercept>`_ (commit f7cebb7 or newer) and its dependencies:

  - `capstone <https://www.capstone-engine.org/>`_ version 4.0.1 or newer.

- `Date <https://github.com/HowardHinnant/date>`_  (commit e7e1482 or newer).

.. important::

    Mercury may need additional plugins depending on the fabric technology used in the cluster. For instance,
    OmniPath requires the `opa-psm2 <https://github.com/cornelisnetworks/opa-psm2>`_ library to work.

Optional dependencies
---------------------

Additionally, some GekkoFS optional execution modes have additional
dependencies:

- `AGIOS <https://github.com/francielizanon/agios>`_ (commit c26a654 or
  newer) to enable the :code:`GekkoFWD` I/O forwarding mode.

- :code:`PARALLAX` There are two different metadata backends in GekkoFS. The default one uses :code:`rocksdb`, however
an alternative based on :code:`PARALLAX` from :code:`FORTH` is available. To enable it, use the
:code:`-DGKFS_ENABLE_PARALLAX:BOOL=ON` option, you can also disable :code:`rocksdb` with
:code:`-DGKFS_ENABLE_ROCKSDB:BOOL=OFF`. Once it is enabled, :code:`--dbbackend` option will be functional.


.. _step_by_step_installation:

Step-by-step installation
=========================

1. Make sure that the :ref:`GekkoFS base dependencies <gkfs_base_dependencies>`
   are available on your machine.

2. Clone GekkoFS:

    .. code-block:: console

      $ git clone --recurse-submodules https://storage.bsc.es/gitlab/hpc/gekkofs.git


   (Optional) If you checked out the sources using :code:`git` without the
   :code`--recursive` option, you need to execute the following command from
   the root of the source directory:

    .. code-block:: console

      $ git submodule update --init

3. Set up the necessary environment variables where the compiled GekkoFS
   :ref:`software dependencies <gkfs_dependencies>` should be installed at.
   Throughout this example we assume dependencies will live in the

   :code:`/home/foo/gekkofs_deps/install` directory):

    .. code-block:: console

       $ export GKFS_INSTALL_PATH=/home/foo/gekkofs_deps/install
       $ export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${GKFS_INSTALL_PATH}/lib:${GKFS_INSTALL_PATH}/lib64

4. Download all the required dependencies using :code:`dl_dep.sh`, one of
   GekkoFS' dependency management scripts. We will use the
   :code:`/home/foo/gekkofs_deps/git` directory to keep all the source files:

    .. code-block:: console

       $ gekkofs/scripts/dl_dep.sh /home/foo/gekkofs_deps/git

.. important::

    The :code:`/home/foo/gekkofs_deps/git` directory containing the source code
    for all our downloaded dependencies can be safely removed once installation
    is complete.

5. Build and install the dependencies into :code:`GKFS_INSTALL_PATH` using
   :code:`compile_dep.sh`, the second GekkoFS' dependency management script:

    .. code-block:: console

       $ gekkofs/scripts/compile_dep.sh /home/foo/gekkofs_deps/git /home/foo/gekkofs_deps/install

6. Now let's configure the GekkoFS build by setting the appropriate options.
   GekkoFS makes use of the CMake build system and requires that you do an
   out-of-source build. In order to do that, you must create a new build
   directory and run the :code:`cmake` command from it:

    .. code-block:: console

       # Create the build directory:
       $ cd gekkofs
       $ mkdir build && cd build
       $ cmake \
            -DCMAKE_BUILD_TYPE:STRING=Release \
            -DCMAKE_PREFIX_PATH:STRING=/home/foo/gekkofs_deps/install \
            -DCMAKE_INSTALL_PREFIX:STRING=/home/foo/gekkofs_deps/install \
            -DGKFS_BUILD_TESTS:BOOL=ON \
            ..


   For this example, we set the :code:`CMAKE_BUILD_TYPE` variable to
   :code:`Release` to let CMake know that we need an optimized build.
   It is very important to make CMake aware of where GekkoFS dependencies are
   installed, which is why we set :code:`CMAKE_PREFIX_PATH` to
   :code:`${GKFS_INSTALL_PATH}`. We also set :code:`CMAKE_INSTALL_PREFIX` to
   the same directory, because we want the GekkoFS binaries to be
   installed in the same location. Finally, we also enable the compilation of
   GekkoFS tests (which are not enabled by default) by setting
   :code:`GKFS_BUILD_TESTS`.

.. attention::
    If you prefer a more interactive approach, it is also possible to use
    :code:`ccmake` or :code:`cmake-gui` to configure the package.

7. We are finally ready to build, test and install GekkoFS:

    .. code-block:: console

       $ make -j8
       $ make test
       $ make install

After following this guide, GekkoFS binaries should now be available in the
appropriate subdirectories of :code:`GKFS_INSTALL_PATH`:

- GekkoFS daemon (server): :code:`${GKFS_INSTALL_PATH}/bin/gkfs_daemon`
- GekkoFS client interception library: :code:`${GKFS_INSTALL_PATH}/lib/libgkfs_intercept.so`

Install GekkoFS via Spack (alternative)
---------------------

Spack is a package manager for supercomputers and Linux. It makes it easy to install scientific software for regular
users. Spack is another method to install GekkoFS where Spack handles all the dependencies and setting up the environment.

Install Spack
=========================

First, install Spack. You can find the instructions here: https://spack.readthedocs.io/en/latest/getting_started.html

    .. code-block:: console
        git clone https://github.com/spack/spack.git
        . spack/share/spack/setup-env.sh

.. attention::
    Note that the second line needs to be executed every time you open a new terminal. It sets up the environment for Spack
and the corresponding environment variables, e.g., $PATH.

Install GekkoFS with Spack
=========================

To install GekkoFS with Spack, the GekkoFS repository needs to be added to Spack as it is not part of the official Spack
repository.

    .. code-block:: console
        spack repo add gekkofs/scripts/spack

When added, the GekkoFS package is available. Its installation variants and options can be checked via:

    .. code-block:: console
        spack info gekkofs

Then install GekkoFS with Spack:

    .. code-block:: console
        spack install gekkofs
        # for installing tests dependencies and running tests
        spack install -v --test=root gekkofs

Finally, GekkoFS is loaded into the currently used environment:

    .. code-block:: console
        spack load gekkofs

This installs the latest release version including its required Git submodules. The installation directory is
:code:`$SPACK_ROOT/opt/spack/linux-<arch>/<compiler>/<version>/gekkofs-<version>`. The GekkoFS daemon (:code:`gkfs_daemon`) is
located in the :code:`bin` directory and the GekkoFS client (:code:`libgkfs_intercept.so`) is located in the :code:`lib` directory.

Note that loading the environment adds the GekkoFS daemon to the `$PATH` environment variable. Therefore, the GekkoFS
daemon is started by running :code:`gkfs_daemon`. Loading GekkoFS in Spack further provides the :code:`$GKFS_CLIENT` environment
variable pointing to the interception library.

Therefore, the following commands can be run to use GekkoFS:

        .. code-block:: console
            # Consult `-h` or the Readme for further options
            gkfs_daemon -r /tmp/gkfs_rootdir -m /tmp/gkfs_mountdir &
            LD_PRELOAD=$GKFS_CLIENT ls -l /tmp/gkfs_mountdir
            LD_PRELOAD=$GKFS_CLIENT touch /tmp/gkfs_mountdir/foo
            LD_PRELOAD=$GKFS_CLIENT ls -l /tmp/gkfs_mountdir

When done using GekkoFS, unload it from the environment:

    .. code-block:: console
        spack unload gekkofs

Miscellaneous
=========================

Use GekkoFS's latest version (master branch) with Spack:

        .. code-block:: console
            spack install gekkofs@latest

Use a specific compiler on your system, e.g., gcc-11.2.0:

        .. code-block:: console
            spack install gekkofs@latest%gcc@11.2.0