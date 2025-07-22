INCLUDEPATH += $$PWD

include($$PWD/tcpServer/tcpServer.pri)

HEADERS += \
    $$PWD/mainserver.h \
    $$PWD/jsonrpcconnectionhandler.h \
    $$PWD/jsonrpcconnectionthread.h \


SOURCES += \
    $$PWD/mainserver.cpp \
    $$PWD/jsonrpcconnectionhandler.cpp \
    $$PWD/jsonrpcconnectionthread.cpp \

