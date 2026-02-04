INCLUDEPATH += $$PWD/../../include

HEADERS += \
    $$PWD/../../include/elib/assert.h \
    $$PWD/../../include/elib/array.h \
    $$PWD/../../include/elib/callback.h \
    $$PWD/../../include/elib/circular_buffer.h \
    $$PWD/../../include/elib/config.h \
    $$PWD/../../include/elib/event_loop.h \
    $$PWD/../../include/elib/expected.h \
    $$PWD/../../include/elib/function_ref.h \
    $$PWD/../../include/elib/inplace_function.h \
    $$PWD/../../include/elib/memory.h \
    $$PWD/../../include/elib/optional.h \
    $$PWD/../../include/elib/scope.h \
    $$PWD/../../include/elib/span.h \
    $$PWD/../../include/elib/stream.h \
    $$PWD/../../include/elib/time/core_clock.h \
    $$PWD/../../include/elib/time/deadline_timer.h \
    $$PWD/../../include/elib/time/elapsed_timer.h \
    $$PWD/../../include/elib/time/system_clock.h \
    $$PWD/../../include/elib/time/timer.h \
    $$PWD/../../include/elib/utility.h \
    $$PWD/../../include/elib/version.h \
    $$PWD/../../include/elib/kernel.h \
    $$PWD/../../include/elib/task.h

SOURCES += \
    $$PWD/../../src/kernel.cpp \
    $$PWD/../../src/task.cpp \
    $$PWD/../../src/time/system_clock.cpp \
    $$PWD/../../src/time/timer.cpp