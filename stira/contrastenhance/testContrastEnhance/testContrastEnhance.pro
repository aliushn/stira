SOURCES += testContrastEnhance.cpp

TEMPLATE = app

LIBS += ../contrastenhance/libcontrastenhance.a \
  ../../histogram/histogram/libhistogram.a \
  ../../filter/filter/libfilter.a \
  ../../fouriertools/fouriertools/libfouriertools.a \
  ../../image/tools/libtools.a \
  ../../image/datastructures/libdatastructures.a \
  ../../image/color/libcolor.a \
  ../../common/common/libcommon.a \
  -lfftw3 \
  -lopencv_highgui \
  -lopencv_core \
  -luuid

POST_TARGETDEPS += ../contrastenhance/libcontrastenhance.a \
  ../../filter/filter/libfilter.a \
  ../../fouriertools/fouriertools/libfouriertools.a \
  ../../histogram/histogram/libhistogram.a \
  ../../image/tools/libtools.a \
  ../../image/datastructures/libdatastructures.a \
  ../../image/color/libcolor.a \
  ../../common/common/libcommon.a

