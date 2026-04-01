# CMake Migration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Migrate `libnexrad` to a Modular Modern CMake build system, supporting out-of-source builds, versioning (0.8.0), and easy downstream integration.

**Architecture:**
1.  **Root Configuration**: Define project and find dependencies (`BZip2`, `ZLIB`).
2.  **Target Abstraction**: Define library and example targets with proper include interfaces.
3.  **Automated Header**: Generate `config.h` from a template into the build directory.
4.  **Cleanup**: Systematically remove legacy Makefiles and `configure` script.

**Tech Stack:** CMake (3.15+), C, BZip2, ZLIB.

---

### Task 1: Root Project & Configuration Template

**Files:**
- Create: `CMakeLists.txt`
- Create: `cmake/config.h.in`

- [ ] **Step 1: Create root `CMakeLists.txt`**

```cmake
cmake_minimum_required(VERSION 3.15)
project(libnexrad VERSION 0.8.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)
find_library(MATH_LIBRARY m)

# Options
option(NEXRAD_BUILD_EXAMPLES "Build example applications" ON)

# Configuration Header
include(CheckIncludeFile)
check_include_file("endian.h" HAVE_ENDIAN_H)
check_include_file("architecture/byte_order.h" HAVE_ARCH_BYTE_ORDER_H)

configure_file(cmake/config.h.in include/nexrad/config.h)

# Subdirectories
add_subdirectory(src)
if(NEXRAD_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
add_subdirectory(colors)
```

- [ ] **Step 2: Create `cmake/config.h.in`**

```c
#ifndef _NEXRAD_CONFIG_H
#define _NEXRAD_CONFIG_H

#cmakedefine HAVE_ENDIAN_H
#cmakedefine HAVE_ARCH_BYTE_ORDER_H

#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif

#ifdef HAVE_ARCH_BYTE_ORDER_H
#include <architecture/byte_order.h>
#ifdef __LITTLE_ENDIAN__
#define __DO_SWAP_BYTES
#endif
#endif

#endif
```

- [ ] **Step 3: Verify configuration**

Run: `cmake -B build`
Expected: Successfully find BZip2/ZLIB and generate `build/include/nexrad/config.h`.

- [ ] **Step 4: Commit**

```bash
git add CMakeLists.txt cmake/config.h.in
git commit -m "feat: add root CMakeLists and config template"
```

---

### Task 2: Library Target Implementation

**Files:**
- Create: `src/CMakeLists.txt`
- Modify: `src/message.c` (to handle new config.h location)

- [ ] **Step 1: Create `src/CMakeLists.txt`**

```cmake
set(NEXRAD_SOURCES
    message.c chunk.c product.c symbology.c graphic.c
    tabular.c packet.c radial.c raster.c image.c
    color.c date.c error.c geo.c poly.c dvl.c
    eet.c util.c pnglite.c geodesic.c level2.c
)

add_library(nexrad SHARED ${NEXRAD_SOURCES})
add_library(nexrad_static STATIC ${NEXRAD_SOURCES})
set_target_properties(nexrad_static PROPERTIES OUTPUT_NAME nexrad)

add_library(nexrad::nexrad ALIAS nexrad)

target_include_directories(nexrad PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(nexrad_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(nexrad PRIVATE BZip2::BZip2 ZLIB::ZLIB ${MATH_LIBRARY})
target_link_libraries(nexrad_static PRIVATE BZip2::BZip2 ZLIB::ZLIB ${MATH_LIBRARY})
```

- [ ] **Step 2: Update `src/message.c` include**

Ensure `#include <nexrad/config.h>` is used instead of `"config.h"`.

- [ ] **Step 3: Compile**

Run: `cmake --build build --target nexrad`
Expected: `libnexrad.so` (or .dylib) exists in `build/src`.

- [ ] **Step 4: Commit**

```bash
git add src/CMakeLists.txt src/message.c
git commit -m "feat: implement nexrad library target in CMake"
```

---

### Task 3: Examples & Colors Subdirectories

**Files:**
- Create: `examples/CMakeLists.txt`
- Create: `colors/CMakeLists.txt`

- [ ] **Step 1: Create `examples/CMakeLists.txt`**

```cmake
set(EXAMPLES
    display drawarc savepng proj
    showproj psychedelic level2_info proj_test
)

foreach(example ${EXAMPLES})
    add_executable(${example} ${example}.c)
    target_link_libraries(${example} PRIVATE nexrad)
endforeach()
```

- [ ] **Step 2: Create `colors/CMakeLists.txt`**

```cmake
add_executable(gencluts gencluts.c)
# Note: Add custom command here if CLUT generation is needed during build
```

- [ ] **Step 3: Build all**

Run: `cmake --build build`
Expected: All examples built and linked against the library.

- [ ] **Step 4: Commit**

```bash
git add examples/CMakeLists.txt colors/CMakeLists.txt
git commit -m "feat: add examples and colors targets to CMake"
```

---

### Task 4: Installation & Package Export

**Files:**
- Modify: `CMakeLists.txt`
- Modify: `src/CMakeLists.txt`

- [ ] **Step 1: Add install rules to `src/CMakeLists.txt`**

```cmake
include(GNUInstallDirs)

install(TARGETS nexrad nexrad_static
    EXPORT nexradTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ../include/nexrad DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../include/nexrad/config.h 
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/nexrad)
```

- [ ] **Step 2: Generate CMake config in root `CMakeLists.txt`**

```cmake
install(EXPORT nexradTargets
    FILE nexradConfig.cmake
    NAMESPACE nexrad::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nexrad
)
```

- [ ] **Step 3: Verify Install**

Run: `cmake --install build --prefix ./dist`
Check: `./dist/include/nexrad` contains all headers.

- [ ] **Step 4: Commit**

```bash
git commit -a -m "feat: add installation and package export rules"
```

---

### Task 5: Cleanup & Git Hygiene

**Files:**
- Modify: `.gitignore`
- Delete: Legacy build files

- [ ] **Step 1: Update `.gitignore`**

```text
/build/
/_build/
/cmake-build-*/
/dist/
compile_commands.json
```

- [ ] **Step 2: Remove legacy files**

```bash
rm Makefile configure
rm src/Makefile examples/Makefile colors/Makefile
rm -rf mk/
rm src/config.h
```

- [ ] **Step 3: Cleanup untracked binaries**

```bash
find src examples -executable -type f -delete
find src examples -name "*.o" -delete
find src examples -name "*.a" -delete
```

- [ ] **Step 4: Final Verification**

Run: `cmake -B build && cmake --build build`
Expected: Clean build from zero state.

- [ ] **Step 5: Commit**

```bash
git add .gitignore
git rm Makefile configure src/Makefile examples/Makefile colors/Makefile
git rm -r mk/
git commit -m "refactor: remove legacy build system and cleanup source tree"
```
