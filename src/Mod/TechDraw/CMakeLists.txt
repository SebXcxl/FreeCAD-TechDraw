
add_definitions(-DMOD_TECHDRAW_HANDLE_FACES=0)

add_subdirectory(App)
if(BUILD_GUI)
    add_subdirectory(Gui)
endif(BUILD_GUI)

INSTALL(
    FILES
        Init.py
        InitGui.py
    DESTINATION
        Mod/TechDraw
)

INSTALL(
    DIRECTORY
        Templates
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}/Mod/TechDraw
    FILES_MATCHING 
        PATTERN "*.svg*"
        PATTERN "*.dxf*"
)
