find_path(MERCURY_UTIL_DIR
        HINTS
        /usr
        /usr/local
        /usr/local/adafs/
        /home/evie/adafs/install
        )

find_library(MERCURY_UTIL_LIBRARY
        NAMES mercury_util
        HINTS
        /home/evie/adafs/install/lib
        /usr
        /usr/local
        /usr/local/adafs
        ${MERCURY_UTIL_DIR}
        )

set(MERCURY_UTIL_LIBRARIES ${MERCURY_UTIL_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Mercury_Util DEFAULT_MSG MERCURY_UTIL_LIBRARY)

mark_as_advanced(
        MERCURY_UTIL_DIR
        MERCURY_UTIL_LIBRARY
        MERCURY_UTIL_INCLUDE_DIR
)