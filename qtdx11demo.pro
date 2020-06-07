#settings for "no qt" win32 app, all WinSDK pathes for libs and includes are setupped!
TEMPLATE = app
CONFIG += c++17
CONFIG -= app_bundle
CONFIG -= qt
QT -= gui

#win32 api libs and dx11
win32: LIBS += -luser32 -lshell32 -lgdi32 -lole32 -ladvapi32 -ld3d11 -ld3dcompiler



# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp

