INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_COMPRESS compress)

FIND_PATH(
    COMPRESS_INCLUDE_DIRS
    NAMES compress/api.h
    HINTS $ENV{COMPRESS_DIR}/include
        ${PC_COMPRESS_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    COMPRESS_LIBRARIES
    NAMES gnuradio-compress
    HINTS $ENV{COMPRESS_DIR}/lib
        ${PC_COMPRESS_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(COMPRESS DEFAULT_MSG COMPRESS_LIBRARIES COMPRESS_INCLUDE_DIRS)
MARK_AS_ADVANCED(COMPRESS_LIBRARIES COMPRESS_INCLUDE_DIRS)

