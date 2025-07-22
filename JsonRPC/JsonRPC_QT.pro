QT       += core gui
QT += network
QT += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
    #LIBS += -L$$PWD/"/3part/lib/win64" -lspdlogd
}

# CONFIG += precompile_header  # 预编译头
# PRECOMPILED_HEADER = precompiled.h

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(./dataBridge/dataBridge.pri)
include(./service/service.pri)
include(./ui/ui.pri)
include(./util/util.pri)


SOURCES += \
    main.cpp \

HEADERS += \
    main.h \
    precompiled.h

FORMS += \



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
