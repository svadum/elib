# Naming Conventions

This library follows the **C++ Standard Library (STL) style**.
Our goal is to feel like a native extension of the language (e.g., `std::vector`, `std::string`).

## General Rules
* **Casing:** Strict `snake_case` is used for almost everything (types, functions, variables).
* **Members:** Private data members have a trailing underscore (`_`).
* **Templates:** Type parameters are `PascalCase` to avoid shadowing standard aliases.

## Summary Table

| Entity | Style | Format | Example |
| :--- | :--- | :--- | :--- |
| **Namespace** | snake_case | `lower_case` | `namespace my_lib` |
| **Type (Class/Struct)** | snake_case | `lower_case` | `class string_view` |
| **Function / Method** | snake_case | `lower_case` | `void calculate_offset()` |
| **Variable / Param** | snake_case | `lower_case` | `int buffer_size` |
| **Private Member** | snake_case + `_` | `lower_case_` | `int capacity_;` |
| **Public Struct Member** | snake_case | `lower_case` | `int x;` |
| **Template Type** | PascalCase | `CamelCase` | `typename ValueType` |
| **Template Non-Type** | snake_case | `lower_case` | `size_t N` |
| **Concept (C++20)** | snake_case | `lower_case` | `concept numeric` |
| **Constant / Enum** | snake_case | `lower_case` | `constexpr double pi` |
| **Macro** | SCREAMING | `UPPER_CASE` | `MY_LIB_VERSION` |
| **File Names** | snake_case | `lower_case.hpp` | `http_client.hpp` |

## Specific Patterns

### Type Aliases
Use standard STL suffixes to avoid shadowing member functions.
* `using value_type = T;`
* `using size_type = std::size_t;`
* `using pointer = T*;`

### Interfaces
Do not use `I` prefixes. Use the noun.
* **Interface:** `class shape`
* **Base/Impl:** `class shape_base` (only if needed for code sharing)
* **Concrete:** `class circle`