SOURCES += testNoiseStatistics.cpp

TEMPLATE = app

LIBS += ../tools/libtools.a \
        ../../imagedata/simpletools/libsimpletools.a \
        ../../imagedata/datastructures/libdatastructures.a \
        ../../imagedata/color/libcolor.a \
        ../../common/common/libcommon.a \
        -lopencv_highgui \
        -lopencv_core

POST_TARGETDEPS += ../tools/libtools.a \
                   ../../imagedata/simpletools/libsimpletools.a \
                   ../../imagedata/color/libcolor.a \
                   ../../imagedata/datastructures/libdatastructures.a \
                   ../../common/common/libcommon.a
