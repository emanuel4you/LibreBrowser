TEMPLATE = app
TARGET = librebrowser
QT += webenginewidgets

HEADERS += \
    bookmark.h \
    librebrowser.h \
    librebrowserwindow.h \
    downloadmanagerwidget.h \
    downloadwidget.h \
    tabwidget.h \
    webpage.h \
    webpopupwindow.h \
    webview.h

SOURCES += \
    bookmark.cpp \
    librebrowser.cpp \
    librebrowserwindow.cpp \
    downloadmanagerwidget.cpp \
    downloadwidget.cpp \
    main.cpp \
    tabwidget.cpp \
    webpage.cpp \
    webpopupwindow.cpp \
    webview.cpp

FORMS += \
    bookmark.ui \
    certificateerrordialog.ui \
    passworddialog.ui \
    downloadmanagerwidget.ui \
    downloadwidget.ui

RESOURCES += \
    data/html.qrc \
    data/data.qrc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/webenginewidgets/librebrowser
INSTALLS += targe

