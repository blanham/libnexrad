# CMake Migration & Build System Upgrade Design

## Purpose
Modernize the `libnexrad` build system by migrating from a legacy `configure` + `Makefile` setup to a **Modular Modern CMake** architecture. This upgrade supports out-of-source builds, improved dependency management, and better integration for downstream projects (via `FetchContent` or submodules).

## Architecture

### 1. Versioning
- **Version**: `0.8.0`
- The project will use semantic versioning to track features and API stability.

### 2. Core Build Structure
The build system will be split into a root `CMakeLists.txt` and directory-specific files:
- **Root**: Project definition, global options, and dependency discovery (`BZip2`, `ZLIB`, `libm`).
- **`src/`**: Library target `nexrad` and alias `nexrad::nexrad`.
- **`examples/`**: Standalone example executables.
- **`colors/`**: Color table generation and installation.

### 3. Automated Configuration
- **Template**: A new `cmake/config.h.in` will replace the shell-based `configure` logic.
- **Generation**: CMake will detect the platform and generate `config.h` in the **build directory**.
- **Result**: No generated artifacts remain in the `src/` directory.

### 4. Developer Experience (DX)
- **Compile Commands**: `CMAKE_EXPORT_COMPILE_COMMANDS` will be enabled by default for LSP support.
- **Out-of-Source**: The library will exclusively support split source/build layouts.
- **Git Hygiene**: `.gitignore` will be updated to ignore all common CMake build artifacts.

### 5. Dependency & Installation
- **Discovery**: Use `find_package(BZip2 REQUIRED)` and `find_package(ZLIB REQUIRED)`.
- **Exporting**: Generate `nexrad-config.cmake` to allow downstream projects to use `find_package(nexrad)`.
- **Targets**: Support `BUILD_INTERFACE` and `INSTALL_INTERFACE` for seamless submodule use.

## Implementation Details

### Cleanup Plan
As part of the migration, the following legacy artifacts will be removed:
- Root `Makefile`, `configure` script.
- `src/Makefile`, `examples/Makefile`, `colors/Makefile`.
- `mk/` directory.
- Untracked binaries in `src/` and `examples/`.
- Stale `src/config.h`.

## Verification Plan
- **Round-Trip Build**: `cmake -B build && cmake --build build` must succeed.
- **Installation**: `cmake --install build --prefix ./dist` must result in a correct directory structure.
- **Downstream Test**: Verify a small test project can consume `libnexrad` via `FetchContent`.
