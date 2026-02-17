[![cmake[windows:cl, ubuntu:gcc]](https://github.com/svadum/elib/actions/workflows/cmake_multiplatform.yml/badge.svg)](https://github.com/svadum/elib/actions/workflows/cmake_multiplatform.yml)

# **elib - C++ library for bare metal embedded systems** 
elib contains a set of useful and embedded-friendly templates, types and utilities:

- Fixed-memory containers: `elib::array`, `elib::circular_buffer`
- Basic event handling: `elib::kernel`, `elib::task`, `elib::generic_task`, `elib::event_loop`
- Time management (`elib::time`): core and system clocks, timers
- Data streams: `elib::data::output_stream`, `elib::data::input_stream`
- Utilities: `elib::aligned_storage`, `elib::scope_exit`

### License
Distributed under the Boost Software License 1.0 – free to use, modify, and distribute.

### Third party licenses

* expected-lite: Copyright by Martin Moene. Boost Software License 1.0. https://github.com/martinmoene/expected-lite
* span-lite:  Copyright by Martin Moene. Boost Software License 1.0. https://github.com/martinmoene/span-lite
* optional-lite: Copyright by Martin Moene. Boost Software License 1.0. https://github.com/martinmoene/optional-lite
