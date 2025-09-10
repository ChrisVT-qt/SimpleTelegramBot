# Where files can be found
INCLUDEPATH += src/
INCLUDEPATH += ../Shared/

# Where files go
OBJECTS_DIR = build/
MOC_DIR = build/

# Frameworks and compiler
TEMPLATE = app
QT += gui
QT += widgets
QT += network
QT += sql
CONFIG += c++17
CONFIG += release
CONFIG += silent

# Don't allow deprecated versions of methods (before Qt 6.8)
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060800

# Shared classes
HEADERS += ../Shared/CallTracer.h
SOURCES += ../Shared/CallTracer.cpp
HEADERS += ../Shared/DatabaseHelper.h
SOURCES += ../Shared/DatabaseHelper.cpp
HEADERS += ../Shared/MD5Sum.h
SOURCES += ../Shared/MD5Sum.cpp
HEADERS += ../Shared/MessageLogger.h
SOURCES += ../Shared/MessageLogger.cpp
HEADERS += ../Shared/StringHelper.h
SOURCES += ../Shared/StringHelper.cpp

# Specific classes
HEADERS += src/Application.h
SOURCES += src/Application.cpp
HEADERS += src/Config.h
HEADERS += src/Deploy.h
SOURCES += src/main.cpp
HEADERS += src/MainWindow.h
SOURCES += src/MainWindow.cpp
HEADERS += src/TelegramComms.h
SOURCES += src/TelegramComms.cpp
HEADERS += src/TelegramHelper.h
SOURCES += src/TelegramHelper.cpp
