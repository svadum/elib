INCLUDEPATH += $$PWD/../include

HEADERS += \
  $$PWD/../include/elib/config.h \
  $$PWD/../include/elib/array.h \
  $$PWD/../include/elib/callback.h \
  $$PWD/../include/elib/circular_buffer.h \
  $$PWD/../include/elib/config.h \
  $$PWD/../include/elib/memory.h \
  $$PWD/../include/elib/span.h \
  $$PWD/../include/elib/version.h \
  $$PWD/../include/elib/time/core_clock.h \
  $$PWD/../include/elib/time/deadline_timer.h \
  $$PWD/../include/elib/time/elapsed_timer.h \
  $$PWD/../include/elib/time/system_clock.h \
  $$PWD/../include/elib/time/timer.h \

SOURCES += \
  $$PWD/../src/time/system_clock.cpp \
  $$PWD/../src/time/timer.cpp \
