# global needed variables

include(ConfigureChecks.cmake)
include(../common/common.cmake)

set(csync_SRCS
  csync.cpp
  csync_exclude.cpp

  std/c_time.cpp

)

if (WIN32)
    list(APPEND csync_SRCS
        vio/csync_vio_local_win.cpp
    )
else()
    list(APPEND csync_SRCS
        vio/csync_vio_local_unix.cpp
    )
endif()

configure_file(csync_version.h.in ${CMAKE_CURRENT_BINARY_DIR}/csync_version.h)

add_library(csync SHARED ${common_SOURCES} ${csync_SRCS})
set_target_properties(csync PROPERTIES EXPORT_NAME CSync)

target_include_directories(csync
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/std
    PUBLIC
        # include root source dir in order to make src/common available to the targets
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

apply_common_target_settings(csync)

if(TOKEN_AUTH_ONLY)
    message("Compiling with token authentication")
    target_compile_definitions(csync PUBLIC -DTOKEN_AUTH_ONLY=1)
endif()

if(NO_MSG_HANDLER)
    target_compile_definitions(csync PUBLIC -DNO_MSG_HANDLER=1)
endif()

if(WIN32)
    target_compile_definitions(csync
        PUBLIC __USE_MINGW_ANSI_STDIO=1
               NOMINMAX
               # Get APIs from from Vista onwards.
               _WIN32_WINNT=0x0600
               WINVER=0x0600
               NTDDI_VERSION=0x0A000003
    )
endif()

generate_export_header(csync
  EXPORT_MACRO_NAME OCSYNC_EXPORT
  EXPORT_FILE_NAME ocsynclib.h
)

target_link_libraries(csync
    PUBLIC
        Qt5::Core
    PRIVATE
        Qt5::Concurrent
        SQLite::SQLite3
        ZLIB::ZLIB
)

# For src/common/utility_mac.cpp
if (APPLE)
    find_library(FOUNDATION_LIBRARY NAMES Foundation)
    find_library(CORESERVICES_LIBRARY NAMES CoreServices)
    target_link_libraries(csync
        PRIVATE
            ${FOUNDATION_LIBRARY} ${CORESERVICES_LIBRARY})
endif()

set_target_properties(
  csync
    PROPERTIES
      OUTPUT_NAME
        "${APPLICATION_EXECUTABLE}_csync"
      VERSION
        ${MIRALL_VERSION}
      SOVERSION
        "0"
)

INSTALL(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/ocsynclib.h
        ../common/filesystembase.h
        ../common/utility.h
    DESTINATION
        ${INCLUDE_INSTALL_DIR}/ownCloud/common
)

INSTALL(TARGETS csync EXPORT ownCloudConfig ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

configure_file(config_csync.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config_csync.h)

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
