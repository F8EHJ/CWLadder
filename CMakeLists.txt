cmake_minimum_required(VERSION 3.19)
project(CWLadder VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network LinguistTools)

qt_standard_project_setup()

set(APP_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/actionicons.h
    src/editlocdialog.cpp
    src/editlocdialog.h
    src/resultsdialog.cpp
    src/resultsdialog.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/helpdialog.cpp
    src/helpdialog.h
    src/qsotypes.h
    src/adifparser.cpp
    src/adifparser.h
    src/adifwriter.cpp
    src/adifwriter.h
    src/countrytable.cpp
    src/countrytable.h
    src/geoloc.cpp
    src/geoloc.h
    src/hamqth.cpp
    src/hamqth.h
    src/mapserver.cpp
    src/mapserver.h
    src/scoring.cpp
    src/scoring.h
    src/appsettings.cpp
    src/appsettings.h
)

# --- Translations -----------------------------------------------------
# Source language is English (embedded directly via tr() calls, no .ts
# needed for it). Each additional language has its own .ts file that is
# compiled into a .qm at build time and embedded as a Qt resource, so the
# application remains a single self-contained executable.
set(TS_FILES
    translations/cwladder_fr.ts
    translations/cwladder_es.ts
    translations/cwladder_it.ts
    translations/cwladder_de.ts
    translations/cwladder_pl.ts
    translations/cwladder_hu.ts
    translations/cwladder_pt.ts
    translations/cwladder_nl.ts
    translations/cwladder_fi.ts
    translations/cwladder_no.ts
    translations/cwladder_sv.ts
)

qt6_add_translation(QM_FILES ${TS_FILES})

qt6_add_executable(CWLadder
    ${APP_SOURCES}
)

qt6_add_resources(CWLadder "data"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/ctyEUCW.csv"
)

# --- Application icon ---------------------------------------------------
# PNGs at several sizes, embedded as resources: used at runtime for the
# window/title-bar icon (QApplication::setWindowIcon in main.cpp), which
# covers the top-left window icon and the taskbar on every platform while
# the app is running.
qt6_add_resources(CWLadder "icons"
    PREFIX "/icons"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon_16.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon_24.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon_32.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon_48.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon_64.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon_128.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/appicon_256.png"
)

# On Windows, also embed the .ico directly into the .exe (via a resource
# script), so Explorer, the taskbar and jump lists show the right icon
# even before Qt has created a window -- setWindowIcon() alone only
# covers the running window.
if(WIN32)
    target_sources(CWLadder PRIVATE resources/app.rc)
endif()

qt6_add_resources(CWLadder "translations"
    PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES ${QM_FILES}
)

target_link_libraries(CWLadder PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

set_target_properties(CWLadder PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

install(TARGETS CWLadder
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
