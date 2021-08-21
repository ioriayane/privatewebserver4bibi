TARGET = privatewebserver4bibi
QT += core httpserver
CONFIG += c++11

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += \
    main.cpp

RESOURCES += \
    bibi_package.qrc

