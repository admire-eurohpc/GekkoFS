FindCargo
---------

Find Cargo include dirs and libraries.

Use this module by invoking find_package with the form::

  find_package(Cargo
    [version] [EXACT]     # Minimum or EXACT version e.g. 0.6.2
    [REQUIRED]            # Fail with error if Cargo is not found
    )

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Cargo::Cargo``
  The Cargo library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``Cargo_FOUND``
  True if the system has the Cargo library.
``Cargo_VERSION``
  The version of the Cargo library which was found.
``Cargo_INCLUDE_DIRS``
  Include directories needed to use Cargo.
``Cargo_LIBRARIES``
  Libraries needed to link to Cargo.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``CARGO_INCLUDE_DIR``
  The directory containing ``cargo.hpp``.
``CARGO_LIBRARY``
  The path to the Cargo library.

#]=======================================================================]

function(_get_pkgconfig_paths target_var)
  set(_lib_dirs)
  if(NOT DEFINED CMAKE_SYSTEM_NAME
     OR (CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU)$"
         AND NOT CMAKE_CROSSCOMPILING)
  )
    if(EXISTS "/etc/debian_version") # is this a debian system ?
      if(CMAKE_LIBRARY_ARCHITECTURE)
        list(APPEND _lib_dirs "lib/${CMAKE_LIBRARY_ARCHITECTURE}/pkgconfig")
      endif()
    else()
      # not debian, check the FIND_LIBRARY_USE_LIB32_PATHS and FIND_LIBRARY_USE_LIB64_PATHS properties
      get_property(uselib32 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB32_PATHS)
      if(uselib32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        list(APPEND _lib_dirs "lib32/pkgconfig")
      endif()
      get_property(uselib64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)
      if(uselib64 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        list(APPEND _lib_dirs "lib64/pkgconfig")
      endif()
      get_property(uselibx32 GLOBAL PROPERTY FIND_LIBRARY_USE_LIBX32_PATHS)
      if(uselibx32 AND CMAKE_INTERNAL_PLATFORM_ABI STREQUAL "ELF X32")
        list(APPEND _lib_dirs "libx32/pkgconfig")
      endif()
    endif()
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" AND NOT CMAKE_CROSSCOMPILING)
    list(APPEND _lib_dirs "libdata/pkgconfig")
  endif()
  list(APPEND _lib_dirs "lib/pkgconfig")
  list(APPEND _lib_dirs "share/pkgconfig")

  set(_extra_paths)
  list(APPEND _extra_paths ${CMAKE_PREFIX_PATH})
  list(APPEND _extra_paths ${CMAKE_FRAMEWORK_PATH})
  list(APPEND _extra_paths ${CMAKE_APPBUNDLE_PATH})

  # Check if directories exist and eventually append them to the
  # pkgconfig path list
  foreach(_prefix_dir ${_extra_paths})
    foreach(_lib_dir ${_lib_dirs})
      if(EXISTS "${_prefix_dir}/${_lib_dir}")
        list(APPEND _pkgconfig_paths "${_prefix_dir}/${_lib_dir}")
        list(REMOVE_DUPLICATES _pkgconfig_paths)
      endif()
    endforeach()
  endforeach()

  set("${target_var}"
      ${_pkgconfig_paths}
      PARENT_SCOPE
  )
endfunction()

# prevent repeating work if the main CMakeLists.txt already called
# find_package(PkgConfig)
if(NOT PKG_CONFIG_FOUND)
  find_package(PkgConfig)
endif()

if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_CARGO QUIET cargo)

  find_path(
    CARGO_INCLUDE_DIR
    NAMES cargo.hpp
    PATHS ${PC_CARGO_INCLUDE_DIRS}
    PATH_SUFFIXES include/cargo
  )

  find_library(
    CARGO_LIBRARY
    NAMES cargo
    PATHS ${PC_CARGO_LIBRARY_DIRS}
  )

  set(Cargo_VERSION ${PC_CARGO_VERSION})
else()
  find_path(
    CARGO_INCLUDE_DIR
    NAMES cargo.hpp
    PATH_SUFFIXES include/cargo
  )

  find_library(CARGO_LIBRARY NAMES cargo)

  # even if pkg-config is not available, but Cargo still installs a .pc file
  # that we can use to retrieve library information from. Try to find it at all
  # possible pkgconfig subfolders (depending on the system).
  _get_pkgconfig_paths(_pkgconfig_paths)

  find_file(_cargo_pc_file cargo.pc PATHS "${_pkgconfig_paths}")

  if(NOT _cargo_pc_file)
    message(
      FATAL_ERROR
        "ERROR: Could not find 'cargo.pc' file. Unable to determine library version"
    )
  endif()

  file(STRINGS "${_cargo_pc_file}" _cargo_pc_file_contents REGEX "Version: ")

  if("${_cargo_pc_file_contents}" MATCHES "Version: ([0-9]+\\.[0-9]+\\.[0-9])")
    set(Cargo_VERSION ${CMAKE_MATCH_1})
  else()
    message(FATAL_ERROR "ERROR: Failed to determine library version")
  endif()

  unset(_pkg_config_paths)
  unset(_cargo_pc_file_contents)
endif()

mark_as_advanced(Cargo_INCLUDE_DIR CARGO_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Cargo
  FOUND_VAR Cargo_FOUND
  REQUIRED_VARS CARGO_LIBRARY CARGO_INCLUDE_DIR
  VERSION_VAR Cargo_VERSION
)

if(Cargo_FOUND)
  set(Cargo_INCLUDE_DIRS ${CARGO_INCLUDE_DIR})
  set(Cargo_LIBRARIES ${CARGO_LIBRARY})
  if(NOT TARGET Cargo::Cargo)
    add_library(Cargo::Cargo UNKNOWN IMPORTED)
    set_target_properties(
      Cargo::Cargo
      PROPERTIES IMPORTED_LOCATION "${CARGO_LIBRARY}"
                 INTERFACE_INCLUDE_DIRECTORIES "${CARGO_INCLUDE_DIR}"
    )
  endif()
endif()
