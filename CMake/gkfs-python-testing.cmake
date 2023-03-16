################################################################################
# Copyright 2018-2022, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2022, Johannes Gutenberg Universitaet Mainz, Germany          #
#                                                                              #
# This software was partially supported by the                                 #
# EC H2020 funded project NEXTGenIO (Project ID: 671951, www.nextgenio.eu).    #
#                                                                              #
# This software was partially supported by the                                 #
# ADA-FS project under the SPPEXA project funded by the DFG.                   #
#                                                                              #
# This file is part of GekkoFS.                                                #
#                                                                              #
# GekkoFS is free software: you can redistribute it and/or modify              #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation, either version 3 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# GekkoFS is distributed in the hope that it will be useful,                   #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with GekkoFS.  If not, see <https://www.gnu.org/licenses/>.            #
#                                                                              #
# SPDX-License-Identifier: GPL-3.0-or-later                                    #
################################################################################

include(CMakeParseArguments)

function(gkfs_enable_python_testing)
    # Parse arguments
    set(MULTI BINARY_DIRECTORIES LIBRARY_PREFIX_DIRECTORIES)

    cmake_parse_arguments(PYTEST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})

    if(PYTEST_UNPARSED_ARGUMENTS)
        message(WARNING "Unparsed arguments in gkfs_enable_python_testing: This often indicates typos!")
    endif()

    if(PYTEST_BINARY_DIRECTORIES)
        set(GKFS_PYTEST_BINARY_DIRECTORIES ${PYTEST_BINARY_DIRECTORIES} PARENT_SCOPE)
    endif()

    if(PYTEST_LIBRARY_PREFIX_DIRECTORIES)
        set(GKFS_PYTEST_LIBRARY_PREFIX_DIRECTORIES ${PYTEST_LIBRARY_PREFIX_DIRECTORIES} PARENT_SCOPE)
    endif()

    set(PYTEST_BINDIR_ARGS, "")
    if(PYTEST_BINARY_DIRECTORIES)
        foreach(dir IN LISTS PYTEST_BINARY_DIRECTORIES)
            list(APPEND PYTEST_BINDIR_ARGS "--bin-dir=${dir}")
        endforeach()
    endif()

    set(PYTEST_LIBDIR_ARGS, "")
    if(PYTEST_LIBRARY_PREFIX_DIRECTORIES)
        foreach(dir IN LISTS PYTEST_LIBRARY_PREFIX_DIRECTORIES)

            if(NOT IS_ABSOLUTE ${dir})
                set(dir ${CMAKE_BINARY_DIR}/${dir})
            endif()

            file(TO_CMAKE_PATH "${dir}/lib" libdir)
            file(TO_CMAKE_PATH "${dir}/lib64" lib64dir)

            if(EXISTS ${libdir})
                list(APPEND PYTEST_LIBDIR_ARGS "--lib-dir=${libdir}")
            endif()

            if(EXISTS ${lib64dir})
                list(APPEND PYTEST_LIBDIR_ARGS "--lib-dir=${lib64dir}")
            endif()
        endforeach()
    endif()

    # convert path lists to space separated arguments
    string(REPLACE ";" " " PYTEST_BINDIR_ARGS "${PYTEST_BINDIR_ARGS}")
    string(REPLACE ";" " " PYTEST_BINDIR_ARGS "${PYTEST_BINDIR_ARGS}")

    configure_file(pytest.ini.in pytest.ini @ONLY)
    configure_file(conftest.py.in conftest.py @ONLY)
    configure_file(harness/cli.py harness/cli.py COPYONLY)

    if(GKFS_INSTALL_TESTS)
        configure_file(pytest.install.ini.in pytest.install.ini @ONLY)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pytest.install.ini
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
            RENAME pytest.ini
        )

        install(FILES conftest.py
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gkfs/tests/integration
        )

        if(NOT PYTEST_VIRTUALENV)
            set(PYTEST_VIRTUALENV ${CMAKE_INSTALL_FULL_DATAROOTDIR}/gkfs/tests/integration/pytest-venv)
        endif()

        # Python's virtual environments are not relocatable, we need to
        # recreate the virtualenv at the appropriate install location
        # find an appropriate python interpreter
        find_package(Python3
            3.6
            REQUIRED
            COMPONENTS Interpreter)

        if(NOT Python3_FOUND)
            message(FATAL_ERROR "Unable to find Python 3")
        endif()

        install(
            CODE "message(\"Install pytest virtual environment...\")"
            CODE "message(\"-- Create virtual environment: ${PYTEST_VIRTUALENV}\")"
            CODE "execute_process(
                    COMMAND ${Python3_EXECUTABLE} -m venv ${PYTEST_VIRTUALENV}
                    RESULT_VARIABLE STATUS
                    OUTPUT_VARIABLE CMD_OUT
                    ERROR_VARIABLE CMD_ERR
                    OUTPUT_FILE install_stdout.log
                    ERROR_FILE install_stderr.log)"
            CODE "if(STATUS AND NOT STATUS EQUAL 0)
                    message(FATAL_ERROR \"Creation of pytest virtual environment failed. Check 'cmake_install_stdout.log' and 'cmake_install_stderr.log' for details\")
                  endif()"
        )

        install(
            CODE "message(\"-- Installing packages...\")"
            CODE "execute_process(
                    COMMAND ${PYTEST_VIRTUALENV}/bin/pip install --upgrade pip -v
                    RESULT_VARIABLE STATUS
                    OUTPUT_VARIABLE CMD_OUT
                    ERROR_VARIABLE CMD_ERR
                    OUTPUT_FILE install_stdout.log
                    ERROR_FILE install_stderr.log)"
            CODE "if(STATUS AND NOT STATUS EQUAL 0)
                    message(FATAL_ERROR \"Installation of pytest dependencies failed. Check 'cmake_install_stdout.log' and 'cmake_install_stderr.log' for details\")
                  endif()"
            CODE "execute_process(
                    COMMAND ${PYTEST_VIRTUALENV}/bin/pip install -r ${CMAKE_CURRENT_BINARY_DIR}/requirements.txt --upgrade -v
                    RESULT_VARIABLE STATUS
                    OUTPUT_VARIABLE CMD_OUT
                    ERROR_VARIABLE CMD_ERR
                    OUTPUT_FILE install_stdout.log
                    ERROR_FILE install_stderr.log)"
            CODE "if(STATUS AND NOT STATUS EQUAL 0)
                    message(FATAL_ERROR \"Installation of pytest dependencies failed. Check 'cmake_install_stdout.log' and 'cmake_install_stderr.log' for details\")
                  endif()"
        )
    endif()

    # enable testing
    set(GKFS_PYTHON_TESTING_ENABLED ON PARENT_SCOPE)

endfunction()

function(gkfs_add_python_test)
    # ignore call if testing is not enabled
    if(NOT CMAKE_TESTING_ENABLED OR NOT GKFS_PYTHON_TESTING_ENABLED)
        return()
    endif()

    # Parse arguments
    set(OPTION)
    set(SINGLE NAME PYTHON_VERSION WORKING_DIRECTORY VIRTUALENV)
    set(MULTI SOURCE BINARY_DIRECTORIES LIBRARY_PREFIX_DIRECTORIES)

    cmake_parse_arguments(PYTEST "${OPTION}" "${SINGLE}" "${MULTI}" ${ARGN})

    if(PYTEST_UNPARSED_ARGUMENTS)
        message(WARNING "Unparsed arguments in gkfs_add_python_test: This often indicates typos!")
    endif()

    if(NOT PYTEST_NAME)
        message(FATAL_ERROR "gkfs_add_python_test requires a NAME argument")
    endif()

    # set default values for arguments not provided
    if(NOT PYTEST_PYTHON_VERSION)
        set(PYTEST_PYTHON_VERSION 3.0)
    endif()

    if(NOT PYTEST_WORKING_DIRECTORY)
        set(PYTEST_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    if(NOT PYTEST_VIRTUALENV)
        set(PYTEST_VIRTUALENV ${CMAKE_CURRENT_BINARY_DIR}/pytest-venv)
    endif()

    # if the test doesn't provide a list of binary or library prefix
    # directories, use the one set on gkfs_enable_python_testing()
    if(NOT PYTEST_BINARY_DIRECTORIES)
        set(PYTEST_BINARY_DIRECTORIES ${GKFS_PYTEST_BINARY_DIRECTORIES})
    endif()

    if(NOT PYTEST_LIBRARY_PREFIX_DIRECTORIES)
        set(PYTEST_LIBRARY_PREFIX_DIRECTORIES ${GKFS_PYTEST_LIBRARY_PREFIX_DIRECTORIES})
    endif()

    set(PYTEST_COMMAND_ARGS, "")
    if(PYTEST_BINARY_DIRECTORIES)
        foreach(dir IN LISTS PYTEST_BINARY_DIRECTORIES)
            list(APPEND PYTEST_COMMAND_ARGS "--bin-dir=${dir}")
        endforeach()
    endif()

    if(PYTEST_LIBRARY_PREFIX_DIRECTORIES)
        foreach(dir IN LISTS PYTEST_LIBRARY_PREFIX_DIRECTORIES)

            if(NOT IS_ABSOLUTE ${dir})
                set(dir ${CMAKE_BINARY_DIR}/${dir})
            endif()

            file(TO_CMAKE_PATH "${dir}/lib" libdir)
            file(TO_CMAKE_PATH "${dir}/lib64" lib64dir)

            if(EXISTS "${dir}/lib")
                list(APPEND PYTEST_COMMAND_ARGS "--lib-dir=${libdir}")
            endif()

            if(EXISTS "${dir}/lib64")
                list(APPEND PYTEST_COMMAND_ARGS "--lib-dir=${lib64dir}")
            endif()
        endforeach()
    endif()

    # Extend the given virtualenv to be a full path.
    if(NOT IS_ABSOLUTE ${PYTEST_VIRTUALENV})
        set(PYTEST_VIRTUALENV ${CMAKE_BINARY_DIR}/${PYTEST_VIRTUALENV})
    endif()

    # find an appropriate python interpreter
    find_package(Python3
        ${PYTEST_PYTHON_VERSION}
        REQUIRED
        COMPONENTS Interpreter)

    set(PYTEST_VIRTUALENV_PIP ${PYTEST_VIRTUALENV}/bin/pip)
    set(PYTEST_VIRTUALENV_INTERPRETER ${PYTEST_VIRTUALENV}/bin/python)

    # create a virtual environment to run the test
    configure_file(requirements.txt.in requirements.txt @ONLY)

    add_custom_command(
        OUTPUT ${PYTEST_VIRTUALENV}
        COMMENT "Creating virtual environment ${PYTEST_VIRTUALENV}"
        COMMAND Python3::Interpreter -m venv "${PYTEST_VIRTUALENV}"
        COMMAND ${PYTEST_VIRTUALENV_PIP} install --upgrade pip -q
        COMMAND ${PYTEST_VIRTUALENV_PIP} install -r requirements.txt --upgrade -q
    )

    if(NOT TARGET venv)
        # ensure that the virtual environment is created by the build process
        # (this is required because we can't add dependencies between
        # "test targets" and "normal targets"
        add_custom_target(venv
            ALL
            DEPENDS ${PYTEST_VIRTUALENV}
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/requirements.txt)
    endif()

    add_test(NAME ${PYTEST_NAME}
             COMMAND ${PYTEST_VIRTUALENV_INTERPRETER}
                    -m pytest -v -s
                    ${PYTEST_COMMAND_ARGS}
                    ${PYTEST_SOURCE}
             WORKING_DIRECTORY ${PYTEST_WORKING_DIRECTORY})

    # instruct Python to not create __pycache__ directories,
    # otherwise they will pollute ${PYTEST_WORKING_DIRECTORY} which
    # is typically ${PROJECT_SOURCE_DIR}
    set_tests_properties(${PYTEST_NAME} PROPERTIES
        ENVIRONMENT PYTHONDONTWRITEBYTECODE=1)

endfunction()
