!contains( CONFIG, c\+\+14 ): warning("UnixSignalsInQt needs at least c++14, add CONFIG += c++14 to your .pro")

CONFIG += exceptions

INCLUDEPATH += $$PWD/UnixSignalsInQt/src

HEADERS += $$PWD/UnixSignalsInQt/src/csystemsignalslistener.h \
    $$PWD/UnixSignalsInQt/src/utils.hp

SOURCES += $$PWD/UnixSignalsInQt/src/csystemsignalslistener.cpp
