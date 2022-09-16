# Use static runtime for all subdirectories
foreach(buildType "" "_DEBUG" "_MINSIZEREL" "_RELEASE" "_RELWITHDEBINFO")
    string(REPLACE "/MD" "/MT" "CMAKE_CXX_FLAGS${buildType}" "${CMAKE_CXX_FLAGS${buildType}}")
endforeach()

add_subdirectory(FMContextMenu)
add_subdirectory(FMOverlays)
add_subdirectory(FMUtil)

