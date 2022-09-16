add_library(FMUtil STATIC
    CommunicationSocket.cpp
    RemotePathChecker.cpp
    StringUtil.cpp
    FMUtil.rc
)

target_include_directories(FMUtil
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
)
