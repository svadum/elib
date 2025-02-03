elib - Embedded Library

elib is a C++17-based library designed for embedded systems development.

### Third-Party Dependencies

1. **span-lite**
   - Used for `span` support when compiling with pre-C++20 compilers.
   - Repository: https://github.com/martinmoene/span-lite
   - License: Boost Software License 1.0 / MIT

### Dependency Handling

- If compiling with C++20 or later, `elib` automatically uses `std::span`.
- If using C++17, `elib` includes a local copy of `span-lite`, so no external dependency management is required.
- `span-lite` is provided as part of the `elib` repository in `include/elib/3rdparty/span-lite/`.

### License Compatibility

- `span-lite` is dual-licensed under the Boost Software License 1.0 and MIT, which makes it compatible with `elib`'s Boost Software License.
- No additional license obligations are required for using `elib`.

### Future Considerations

- Additional dependencies may be introduced in the future, following the same inclusion and licensing policies.
- If `elib` is used in a project with CMake, external dependencies can be overridden via `FetchContent` or custom configurations.
