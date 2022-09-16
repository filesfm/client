add_library (FMOverlays MODULE
    DllMain.cpp
    FMOverlay.cpp
    FMOverlayFactory.cpp
    FMOverlayRegistrationHandler.cpp
    FMOverlay.rc
    FMOverlays.def
)

target_link_libraries(FMOverlays
    FMUtil)

install(TARGETS FMOverlays
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)
