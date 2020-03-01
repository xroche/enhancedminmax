# Enhanced `std::min` and `std::max` versions

## Why ?

Currently, the standard C++ `std::min` and `std::max` are rather limited, and lack some features that are now possible with enablement of new C++17 features:

* Handling more than two elements to be compared
* Handling different signedness
* Returning references when all elements have suitable types
* All the former being `constexpr`-compatible

## Example

* Handle more than two elements
```c++
static_assert(enhancedminmax::max(0, 1, 2, 3, 4, 5) == 5);
```

* Handling different signedness
```c++
static_assert(enhancedminmax::min(-2, 0u, 7u) == -2);
```

* Returning references when possible
```c++
    std::size_t a = 42;
    std::size_t b = 100;
    ++enhancedminmax::max(a, b);
```

## How ?

- Among the magic allowed by C++17 is the ability to return references in automatic types, using `decltype(auto)`. This allows to pass references, based statically on provided types at compile-time.
- Handling integral types with different signedness is more challenging: `true ? -1 : 1` yields an unsigned type, making further comparisons erroneous. To solve that, we need to pass the original signedness when handling integral types.

## Build

```shell
mkdir -p build && (cd build && cmake -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..)
(cd build && ninja)
```
