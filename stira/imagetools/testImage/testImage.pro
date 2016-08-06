

TEMPLATE = app

LIBS += ../tools/libtools.a \
        ../imagegenerator/libimagegenerator.a \
        ../../imagedata/simpletools/libsimpletools.a \
        ../../imagedata/datastructures/libdatastructures.a \
        ../../imagedata/color/libcolor.a \
        ../../common/common/libcommon.a \
        -lopencv_highgui \
        -lopencv_core

POST_TARGETDEPS += ../tools/libtools.a \
                   ../imagegenerator/libimagegenerator.a \
                   ../../imagedata/simpletools/libsimpletools.a \
                   ../../imagedata/datastructures/libdatastructures.a \
                   ../../imagedata/color/libcolor.a \
                   ../../common/common/libcommon.a

SOURCES += testImage.cpp
