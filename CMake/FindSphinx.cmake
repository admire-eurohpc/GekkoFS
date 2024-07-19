################################################################################
# Copyright 2018-2024, Barcelona Supercomputing Center (BSC), Spain            #
# Copyright 2015-2024, Johannes Gutenberg Universitaet Mainz, Germany          #
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

# Heavily based on FindSphinx.cmake (https://github.com/k0ekk0ek/cmake-sphinx)

#[=======================================================================[.rst:
FindSphinx
----------

Sphinx is a documentatino generation tool (see https://www.sphinx-doc.org).
This module looks for Sphinx and some optional extensions in order to connect
it to Doxygen documentation. These tools are enabled as components in the
:command:`find_package` command:

``breathe``
  `Breathe <https://breathe.readthedocs.io>`_ A utility that provides a bridge
  between the Sphinx and Doxygen documentation systems.

Examples:

.. code-block:: cmake

  find_package(Sphinx
               REQUIRED breathe
               OPTIONAL_COMPONENTS exhale)

The following variables are defined by this module:

.. variable:: SPHINX_FOUND

  True if the ``sphinx`` executable was found.

.. variable:: SPHINX_BUILD_VERSION

  The version reported by ``sphinx-build --version``.


Functions
^^^^^^^^^

.. command:: sphinx_add_docs

  This function is intended as a convenience for adding a target for generating
  documentation with Sphinx. It aims to provide sensible defaults so that
  projects can generally just provide the input files and directories and that
  will be sufficient to give sensible results.

  ::

    sphinx_add_docs(targetName
        [filesOrDirs...]
        [ALL]
        [BUILDER builder]
        [SOURCE_DIRECTORY dir]
        [OUTPUT_DIRECTORY dir]
        [BREATHE_PROJECTS doxygenTargetName]
        [COMMENT comment])

  The function defines a custom target that runs Sphinx on all plain-text
  document sources found in ``SOURCE_DIRECTORY``, including its subdirectories.
  The documentation generated will be placed in the directory defined by 
  the ``OUTPUT_DIRECTORY`` option (or in
  `${CMAKE_CURRENT_BINARY_DIR}/targetName`, if the option is not provided).
  
  The ``CONF_TEMPLATE`` option allows users to provide a template for Sphinx's
  configuration file ``conf.py``. This template will be copied into
  ``${CMAKE_CURRENT_BINARY_DIR}/targetName.cache/_build`` using CMake's
  `` configure_file()``, which means that any CMake variable can be used
  in the template and will be replaced by with its corresponding value
  at the time of invocation (see the `configure_file() documentation
  <https://cmake.org/cmake/help/latest/command/configure_file.html>`_) for more
  details).

  .. variable:: SPHINX_EXTENSIONS
  .. variable:: SPHINX_EXTENSIONS

  The ``BUILDER`` option allows controlling the Sphinx builder that should be 
  used when producing the documentation, e.g. html or man, among others. Please 
  refer to Sphinx's documentation for the full list.

  If ``BREATHE_PROJECTS`` is set, the function will use the Breathe extension to 
  include already-generated Doxygen documentation into the produced Sphinx
  documentation. ``<doxygenTargetName>`` must thus be a target that produces
  such documentation, such as the one created with ``doxygen_add_docs()`` in the
  ``FindDoxygen.cmake`` module.

  If provided, the optional ``comment`` will be passed as the ``COMMENT`` for
  the :command:`add_custom_target` command used to create the custom target
  internally.

  If ``ALL`` is set, the target will be added to the default build target.

  The contents of the generated ``conf.py`` for Sphinx can be customized by
  setting CMake variables before calling ``sphinx_add_docs()``. Each of the
  following will be explictly set unless the variable already has a value
  before ``sphinx_add_docs()`` is called:

  .. variable:: EXHALE_CONTAINMENT_FOLDER

    This variable overrides the ``containmentFolder`` key in the
    ``exhale_args`` dictionary (see the `Exhale documentation 
    <https://exhale.readthedocs.io/en/latest/reference/configs.html>`_ for
    details). Set to ``api`` by this module.

  .. variable:: EXHALE_ROOT_FILE_NAME

    This variable overrides the ``rootFileName`` key in the 
    ``exhale_args`` dictionary (see the `Exhale documentation 
    <https://exhale.readthedocs.io/en/latest/reference/configs.html>`_ for
    details).
    Set to ``api.rst`` by this module.

  .. variable:: EXHALE_ROOT_FILE_TITLE

    This variable overrides the ``rootFileTitle`` key in the 
    ``exhale_args`` dictionary (see the `Exhale documentation 
    <https://exhale.readthedocs.io/en/latest/reference/configs.html>`_ for
    details).
    Set to ``${PROJECT_NAME} Reference`` by this module.

  To change any of these defaults, set relevant variables before calling
  ``sphinx_add_docs()``. For example:

    .. code-block:: cmake

      set(EXHALE_CONTAINMENT_FOLDER api)
      set(EXHALE_ROOT_FILE_NAME reference.rst)
      set(EXHALE_ROOT_FILE_TITLE "Reference")

      sphinx_add_docs(
          sphinx_docs
          CONF_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in"
          BREATHE_PROJECTS doxygen_docs
          BUILDER html
          SOURCE_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/sphinx
          COMMENT "Generating Sphinx+Breathe documentation"
      )


#]=======================================================================]

include(FindPackageHandleStandardArgs)

macro(_Sphinx_find_executable _exe)
    string(TOUPPER "${_exe}" _uc)
    # sphinx-(build|quickstart)-3 x.x.x
    # FIXME: This works on Fedora (and probably most other UNIX like targets).
    #        Windows targets and PIP installs might need some work.
    find_program(
        SPHINX_${_uc}_EXECUTABLE
        NAMES "sphinx-${_exe}-3" "sphinx-${_exe}" "sphinx-${_exe}.exe")

    if (SPHINX_${_uc}_EXECUTABLE)
        execute_process(
            COMMAND "${SPHINX_${_uc}_EXECUTABLE}" --version
            RESULT_VARIABLE _result
            OUTPUT_VARIABLE _output
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (_result EQUAL 0 AND _output MATCHES " v?([0-9]+\\.[0-9]+\\.[0-9]+)$")
            set(SPHINX_${_uc}_VERSION "${CMAKE_MATCH_1}")
        endif ()

        if (NOT TARGET Sphinx::${_exe})
            add_executable(Sphinx::${_exe} IMPORTED GLOBAL)
            set_target_properties(Sphinx::${_exe} PROPERTIES
                IMPORTED_LOCATION "${SPHINX_${_uc}_EXECUTABLE}")
        endif ()
        set(Sphinx_${_exe}_FOUND TRUE)
    else ()
        set(Sphinx_${_exe}_FOUND FALSE)
    endif ()
    unset(_uc)
endmacro()

macro(_Sphinx_find_module _name _module)
    string(TOUPPER "${_name}" _Sphinx_uc)
    if (SPHINX_PYTHON_EXECUTABLE)
        execute_process(
            COMMAND ${SPHINX_PYTHON_EXECUTABLE} -m ${_module} --version
            RESULT_VARIABLE _result
            OUTPUT_VARIABLE _output
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET)
        if (_result EQUAL 0)
            if (_output MATCHES " v?([0-9]+\\.[0-9]+\\.[0-9]+)$")
                set(SPHINX_${_Sphinx_uc}_VERSION "${CMAKE_MATCH_1}")
            endif ()

            if (NOT TARGET Sphinx::${_name})
                set(SPHINX_${_Sphinx_uc}_EXECUTABLE "${SPHINX_PYTHON_EXECUTABLE} -m ${_module}")
                add_executable(Sphinx::${_name} IMPORTED GLOBAL)
                set_target_properties(Sphinx::${_name} PROPERTIES
                    IMPORTED_LOCATION "${SPHINX_PYTHON_EXECUTABLE}")
            endif ()
            set(Sphinx_${_name}_ARGS -m ${_module})
            set(Sphinx_${_name}_FOUND TRUE)
        else ()
            set(Sphinx_${_name}_FOUND FALSE)
        endif ()
    else ()
        set(Sphinx_${_name}_FOUND FALSE)
    endif ()
    unset(_Sphinx_uc)
endmacro()

macro(_Sphinx_find_extension _ext)
    if (SPHINX_PYTHON_EXECUTABLE)
        execute_process(
            COMMAND ${SPHINX_PYTHON_EXECUTABLE} -c "import ${_ext}"
            RESULT_VARIABLE _result)
        if (_result EQUAL 0)
            set(Sphinx_${_ext}_FOUND TRUE)
        else ()
            set(Sphinx_${_ext}_FOUND FALSE)
        endif ()
    endif ()
endmacro()

#
# Find sphinx-build and sphinx-quickstart.
#

# Find sphinx-build shim.
_Sphinx_find_executable(build)

if (SPHINX_BUILD_EXECUTABLE)
    # Find sphinx-quickstart shim.
    _Sphinx_find_executable(quickstart)

    # Locate Python executable
    if (CMAKE_HOST_WIN32)
        # script-build on Windows located under (when PIP is used):
        # C:/Program Files/PythonXX/Scripts
        # C:/Users/username/AppData/Roaming/Python/PythonXX/Sripts
        #
        # Python modules are installed under:
        # C:/Program Files/PythonXX/Lib
        # C:/Users/username/AppData/Roaming/Python/PythonXX/site-packages
        #
        # To verify a given module is installed, use the Python base directory
        # and test if either Lib/module.py or site-packages/module.py exists.
        get_filename_component(_Sphinx_directory "${SPHINX_BUILD_EXECUTABLE}" DIRECTORY)
        get_filename_component(_Sphinx_directory "${_Sphinx_directory}" DIRECTORY)
        if (EXISTS "${_Sphinx_directory}/python.exe")
            set(SPHINX_PYTHON_EXECUTABLE "${_Sphinx_directory}/python.exe")
        endif ()
        unset(_Sphinx_directory)
    else ()
        file(READ "${SPHINX_BUILD_EXECUTABLE}" _Sphinx_script)
        if (_Sphinx_script MATCHES "^#!([^\n]+)")
            string(STRIP "${CMAKE_MATCH_1}" _Sphinx_shebang)
            if (EXISTS "${_Sphinx_shebang}")
                set(SPHINX_PYTHON_EXECUTABLE "${_Sphinx_shebang}")
            endif ()
        endif ()
        unset(_Sphinx_script)
        unset(_Sphinx_shebang)
    endif ()
endif ()

if (NOT SPHINX_PYTHON_EXECUTABLE)
    # Python executable cannot be extracted from shim shebang or path if e.g.
    # virtual environments are used, fallback to find package. Assume the
    # correct installation is found, the setup is probably broken in more ways
    # than one otherwise.
    find_package(Python3 QUIET COMPONENTS Interpreter)
    if (TARGET Python3::Interpreter)
        set(SPHINX_PYTHON_EXECUTABLE ${Python3_EXECUTABLE})
        # Revert to "python -m sphinx" if shim cannot be found.
        if (NOT SPHINX_BUILD_EXECUTABLE)
            _Sphinx_find_module(build sphinx)
            _Sphinx_find_module(quickstart sphinx.cmd.quickstart)
        endif ()
    endif ()
endif ()

#
# Verify components are available.
#
if (SPHINX_BUILD_VERSION)

    # Breathe is required for Exhale
    if ("exhale" IN_LIST Sphinx_FIND_COMPONENTS AND NOT
        "breathe" IN_LIST Sphinx_FIND_COMPONENTS)
        list(APPEND Sphinx_FIND_COMPONENTS "breathe")
    endif ()

    # Find all requested components for Sphinx...
    foreach (_Sphinx_component IN LISTS Sphinx_FIND_COMPONENTS)
        if (_Sphinx_component STREQUAL "build")
            # Do nothing, sphinx-build is always required.
            continue()
        elseif (_Sphinx_component STREQUAL "quickstart")
            # Do nothing, sphinx-quickstart is optional, but looked up by default.
            continue()
        endif ()
        _Sphinx_find_extension(${_Sphinx_component})
    endforeach ()
    unset(_Sphinx_component)

    #
    # Verify both executables are part of the Sphinx distribution.
    #
    if (SPHINX_QUICKSTART_VERSION AND NOT SPHINX_BUILD_VERSION STREQUAL SPHINX_QUICKSTART_VERSION)
        message(FATAL_ERROR "Versions for sphinx-build (${SPHINX_BUILD_VERSION}) "
            "and sphinx-quickstart (${SPHINX_QUICKSTART_VERSION}) "
            "do not match")
    endif ()
endif ()

find_package_handle_standard_args(
    Sphinx
    VERSION_VAR SPHINX_BUILD_VERSION
    REQUIRED_VARS SPHINX_BUILD_EXECUTABLE SPHINX_BUILD_VERSION
    HANDLE_COMPONENTS)

# Generate a conf.py template file using sphinx-quickstart.
#
# sphinx-quickstart allows for quiet operation and a lot of settings can be
# specified as command line arguments, therefore its not required to parse the
# generated conf.py.
function(_Sphinx_generate_confpy _target _cachedir)
    if (NOT TARGET Sphinx::quickstart)
        message(FATAL_ERROR "sphinx-quickstart is not available, needed by"
            "sphinx_add_docs for target ${_target}")
    endif ()

    if (NOT DEFINED SPHINX_PROJECT)
        set(SPHINX_PROJECT ${PROJECT_NAME})
    endif ()

    if (NOT DEFINED SPHINX_AUTHOR)
        set(SPHINX_AUTHOR "${SPHINX_PROJECT} committers")
    endif ()

    if (NOT DEFINED SPHINX_COPYRIGHT)
        string(TIMESTAMP "%Y, ${SPHINX_AUTHOR}" SPHINX_COPYRIGHT)
    endif ()

    if (NOT DEFINED SPHINX_VERSION)
        set(SPHINX_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
    endif ()

    if (NOT DEFINED SPHINX_RELEASE)
        set(SPHINX_RELEASE "${PROJECT_VERSION}")
    endif ()

    if (NOT DEFINED SPHINX_LANGUAGE)
        set(SPHINX_LANGUAGE "en")
    endif ()

    if (NOT DEFINED SPHINX_MASTER)
        set(SPHINX_MASTER "index")
    endif ()

    set(_known_exts autodoc doctest intersphinx todo coverage imgmath mathjax
        ifconfig viewcode githubpages)

    if (DEFINED SPHINX_EXTENSIONS)
        foreach (_ext ${SPHINX_EXTENSIONS})
            set(_is_known_ext FALSE)
            foreach (_known_ext ${_known_exsts})
                if (_ext STREQUAL _known_ext)
                    set(_opts "${opts} --ext-${_ext}")
                    set(_is_known_ext TRUE)
                    break()
                endif ()
            endforeach ()
            if (NOT _is_known_ext)
                if (_exts)
                    set(_exts "${_exts},${_ext}")
                else ()
                    set(_exts "${_ext}")
                endif ()
            endif ()
        endforeach ()
    endif ()

    if (_exts)
        set(_exts "--extensions=${_exts}")
    endif ()

    set(_templatedir "${CMAKE_CURRENT_BINARY_DIR}/${_target}.template")
    file(MAKE_DIRECTORY "${_templatedir}")
    string(REPLACE " " ";" _Sphinx_executable ${SPHINX_QUICKSTART_EXECUTABLE})
    execute_process(
        COMMAND ${_Sphinx_executable}
        -q --no-makefile --no-batchfile
        -p "${SPHINX_PROJECT}"
        -a "${SPHINX_AUTHOR}"
        -v "${SPHINX_VERSION}"
        -r "${SPHINX_RELEASE}"
        -l "${SPHINX_LANGUAGE}"
        --master "${SPHINX_MASTER}"
        ${_opts} ${_exts} "${_templatedir}"
        RESULT_VARIABLE _result
        OUTPUT_QUIET)
    unset(_Sphinx_executable)

    if (_result EQUAL 0 AND EXISTS "${_templatedir}/conf.py")
        file(COPY "${_templatedir}/conf.py" DESTINATION "${_cachedir}")
    endif ()

    file(REMOVE_RECURSE "${_templatedir}")

    if (NOT _result EQUAL 0 OR NOT EXISTS "${_cachedir}/conf.py")
        message(FATAL_ERROR "Sphinx configuration file not generated for "
            "target ${_target}")
    endif ()
endfunction()

function(sphinx_add_docs _target)
    set(_opts ALL)
    set(_single_opts CONF_TEMPLATE BUILDER OUTPUT_DIRECTORY SOURCE_DIRECTORY COMMENT)
    set(_multi_opts BREATHE_PROJECTS)
    cmake_parse_arguments(_args "${_opts}" "${_single_opts}" "${_multi_opts}" ${ARGN})

    unset(SPHINX_BREATHE_PROJECTS)

    if (NOT _args_COMMENT)
        set(_args_COMMENT "Generating Sphinx API documentation for ${_target}")
    endif ()

    if (_args_CONF_TEMPLATE AND NOT EXISTS ${_args_CONF_TEMPLATE})
        message(FATAL_ERROR "CONF_TEMPLATE file '${_args_CONF_TEMPLATE}' does not exist")
    endif ()

    if (NOT EXHALE_CONTAINMENT_FOLDER)
        set(EXHALE_CONTAINMENT_FOLDER "api")
    endif ()

    if (NOT EXHALE_ROOT_FILE_NAME)
        set(EXHALE_ROOT_FILE_NAME "api.rst")
    endif ()

    if (NOT EXHALE_ROOT_FILE_TITLE)
        set(EXHALE_ROOT_FILE_TITLE "${PROJECT_NAME} Reference")
    endif ()

    if (NOT _args_BUILDER)
        message(FATAL_ERROR "Sphinx builder not specified for target ${_target}")
    elseif (NOT _args_SOURCE_DIRECTORY)
        message(FATAL_ERROR "Sphinx source directory not specified for target ${_target}")
    else ()
        if (NOT IS_ABSOLUTE "${_args_SOURCE_DIRECTORY}")
            get_filename_component(_sourcedir "${_args_SOURCE_DIRECTORY}" ABSOLUTE)
        else ()
            set(_sourcedir "${_args_SOURCE_DIRECTORY}")
        endif ()
        if (NOT IS_DIRECTORY "${_sourcedir}")
            message(FATAL_ERROR "Sphinx source directory '${_sourcedir}' for"
                "target ${_target} does not exist")
        endif ()
    endif ()

    set(_builder "${_args_BUILDER}")
    if (_args_OUTPUT_DIRECTORY)
        set(_outputdir "${_args_OUTPUT_DIRECTORY}")
    else ()
        set(_outputdir "${CMAKE_CURRENT_BINARY_DIR}/${_target}")
    endif ()

    if (_args_BREATHE_PROJECTS)
        if (NOT Sphinx_breathe_FOUND)
            message(FATAL_ERROR "Sphinx extension 'breathe' is not available. Needed"
                "by sphinx_add_docs for target ${_target}")
        endif ()
        list(APPEND SPHINX_EXTENSIONS breathe)

        foreach (_doxygen_target ${_args_BREATHE_PROJECTS})
            if (TARGET ${_doxygen_target})
                list(APPEND _depends ${_doxygen_target})

                # Doxygen targets are supported. Verify that a Doxyfile exists.
                get_target_property(_dir ${_doxygen_target} BINARY_DIR)
                set(_doxyfile "${_dir}/Doxyfile.${_doxygen_target}")
                if (NOT EXISTS "${_doxyfile}")
                    message(FATAL_ERROR "Target ${_doxygen_target} is not a Doxygen"
                        "target, needed by sphinx_add_docs for target"
                        "${_target}")
                endif ()

                # Read the Doxyfile, verify XML generation is enabled and retrieve the
                # output directory.
                file(READ "${_doxyfile}" _contents)
                if (NOT _contents MATCHES "GENERATE_XML *= *YES")
                    message(FATAL_ERROR "Doxygen target ${_doxygen_target} does not"
                        "generate XML, needed by sphinx_add_docs for"
                        "target ${_target}")
                elseif (_contents MATCHES "OUTPUT_DIRECTORY *= *([^ ][^\n]*)")
                    string(STRIP "${CMAKE_MATCH_1}" _dir)
                    set(_name "${_doxygen_target}")
                    set(_dir "${_dir}/xml")
                else ()
                    message(FATAL_ERROR "Cannot parse Doxyfile generated by Doxygen"
                        "target ${_doxygen_target}, needed by"
                        "sphinx_add_docs for target ${_target}")
                endif ()
            elseif (_doxygen_target MATCHES "([^: ]+) *: *(.*)")
                set(_name "${CMAKE_MATCH_1}")
                string(STRIP "${CMAKE_MATCH_2}" _dir)
            endif ()

            if (_name AND _dir)
                if (_breathe_projects)
                    set(_breathe_projects "${_breathe_projects}, \"${_name}\": \"${_dir}\"")
                else ()
                    set(_breathe_projects "\"${_name}\": \"${_dir}\"")
                endif ()
                if (NOT _breathe_default_project)
                    set(_breathe_default_project "${_name}")
                endif ()
            endif ()
        endforeach ()
    endif ()

    if (Sphinx_exhale_FOUND)
        if (NOT Sphinx_breathe_FOUND)
            message(FATAL_ERROR "Sphinx extension 'breathe' is not available, but is "
                "required by exhale. Needed by sphinx_add_docs for "
                "target ${_target}")
        endif ()
        list(APPEND SPHINX_EXTENSIONS exhale)
    endif ()

    set(_cachedir "${CMAKE_CURRENT_BINARY_DIR}/${_target}.cache")
    set(_cached_sourcesdir "${_cachedir}/_sources")
    set(_cached_builddir "${_cachedir}/_build")
    file(MAKE_DIRECTORY "${_cachedir}")
    file(MAKE_DIRECTORY "${_cached_sourcesdir}")
    file(MAKE_DIRECTORY "${_cached_builddir}")
    file(MAKE_DIRECTORY "${_cached_builddir}/_static")

    # prepare Sphinx configuration file
    set(_target_sphinx_conf "${_cached_builddir}/conf.py")
    if (_args_CONF_TEMPLATE)
        set(_sphinx_conf_template ${_args_CONF_TEMPLATE})

        list(TRANSFORM SPHINX_EXTENSIONS REPLACE "^(.+)$" "'\\1'" OUTPUT_VARIABLE _foo)
        list(JOIN _foo ",\n    " SPHINX_EXTENSIONS)
        unset(_foo)

        configure_file("${_sphinx_conf_template}" "${_target_sphinx_conf}" @ONLY)
    else ()
        _Sphinx_generate_confpy(${_target} "${_cached_builddir}")
    endif ()

    if (_breathe_projects)
        file(APPEND "${_target_sphinx_conf}"
            "\n# Setup the breathe extension"
            "\nbreathe_projects = { ${_breathe_projects} }"
            "\nbreathe_default_project = '${_breathe_default_project}'"
            "\n")
    endif ()

    if (Sphinx_exhale_FOUND)
        set(_exhale_containment_folder "${_cached_sourcesdir}/${EXHALE_CONTAINMENT_FOLDER}")
        set(_exhale_root_file_name ${EXHALE_ROOT_FILE_NAME})
        set(_exhale_root_file_title "${EXHALE_ROOT_FILE_TITLE}")

        file(APPEND "${_target_sphinx_conf}"
            "\n# Setup the exhale extension"
            "\nfrom exhale import utils"
            "\nexhale_args = {"
            "\n    'containmentFolder': '${_exhale_containment_folder}',"
            "\n    'rootFileName': '${_exhale_root_file_name}',"
            "\n    'rootFileTitle': '${_exhale_root_file_title}',"
            "\n    'doxygenStripFromPath': '${CMAKE_SOURCE_DIR}',"
            "\n    'createTreeView': True,"
            "\n    'customSpecificationsMapping': utils.makeCustomSpecificationsMapping(specificationsForKind)"
            "\n}")
    endif ()

    unset(_all)
    if (${_args_ALL})
        set(_all ALL)
    endif ()

    # Propagate some variables
    set(SPHINX_OUTPUT_DIRECTORY "${_outputdir}" PARENT_SCOPE)

    string(REPLACE " " ";" _Sphinx_executable ${SPHINX_BUILD_EXECUTABLE})
    add_custom_target(
        ${_target} ${_all}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${_sourcedir} ${_cached_sourcesdir}
        COMMAND ${_Sphinx_executable}
        -b ${_builder}
        -d "${_cached_builddir}/_doctrees"
        -c "${_cached_builddir}"
        "${_cached_sourcesdir}"
        "${_outputdir}"
        DEPENDS "${_target_sphinx_conf}" ${_depends}
        COMMENT "${_args_COMMENT}")
    unset(_Sphinx_executable)

    list(APPEND
        _sphinx_generated_files
        ${_cached_builddir}/_doctrees
        ${_cached_builddir}/_static
        ${_cached_sourcesdir}
    )

    set_target_properties(${_target}
        PROPERTIES ADDITIONAL_CLEAN_FILES "${_sphinx_generated_files}")

endfunction()
