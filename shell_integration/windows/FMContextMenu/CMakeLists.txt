add_library(FMContextMenu MODULE
    dllmain.cpp
    FMClientInterface.cpp
    FMContextMenu.cpp
    FMContextMenuFactory.cpp
    FMContextMenuRegHandler.cpp
    FMContextMenu.rc
    FMContextMenu.def
)

target_link_libraries(FMContextMenu
                      FMUtil
                      crypt32
                      gdiplus)
target_compile_definitions(FMContextMenu PRIVATE JSON_NOEXCEPTION)

install(TARGETS FMContextMenu
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)
