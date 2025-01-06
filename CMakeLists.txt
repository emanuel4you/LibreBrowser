cmake_minimum_required(VERSION 3.16)
project(librebrowser LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/webenginewidgets/librebrowser")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS WebEngineWidgets)

qt_add_executable(librebrowser
    librebrowser.cpp librebrowser.h
    librebrowserwindow.cpp librebrowserwindow.h
    certificateerrordialog.ui
    downloadmanagerwidget.cpp downloadmanagerwidget.h downloadmanagerwidget.ui
    downloadwidget.cpp downloadwidget.h downloadwidget.ui
    main.cpp
    passworddialog.ui
    tabwidget.cpp tabwidget.h
    webpage.cpp webpage.h
    webpopupwindow.cpp webpopupwindow.h
    webview.cpp webview.h
)
set_target_properties(librebrowser PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(librebrowser PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)


# Resources:
set(librebrowser_resource_files
    "data/AppLogoColor.png"
    "data/ninja.png"
)

qt_add_resources(librebrowser "librebrowser"
    PREFIX
        "/"
    BASE
        "data"
    FILES
        ${librebrowser_resource_files}
)
set(librebrowser1_resource_files
    "data/3rdparty/dialog-error.png"
    "data/3rdparty/edit-clear.png"
    "data/3rdparty/go-bottom.png"
    "data/3rdparty/go-next.png"
    "data/3rdparty/go-previous.png"
    "data/3rdparty/process-stop.png"
    "data/3rdparty/text-html.png"
    "data/3rdparty/view-refresh.png"
)

qt_add_resources(librebrowser "librebrowser1"
    PREFIX
        "/"
    BASE
        "data/3rdparty"
    FILES
        ${librebrowser1_resource_files}
)

install(TARGETS librebrowser
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
