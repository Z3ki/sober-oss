# Sober Reverse Engineering & Open Source Documentation

Reverse engineering project for [Sober](https://sober.vinegarhq.org) by VinegarHQ. Sober is a proprietary Linux runtime that runs the Roblox Android APK natively on x86 64 Linux via binary translation. The developers chose to close source it to reduce abuse potential (which could get Roblox to block them again).

This repo provides decompiled pseudocode, architecture documentation, and analysis notes so the community can understand, audit, and potentially reimplement the software.

## Architecture Overview

Sober consists of 4 main proprietary binaries distributed via Flatpak:

| Binary | Size | Language | Purpose |
|---|---|---|---|
| `sober` | 7.1 MB | Rust | Main game runtime. Handles Android binary translation, OpenGL/Vulkan rendering, process management, networking. Links to libloader.so, libbadcpu.so, libmimalloc.so, libcrypto, libcurl, libEGL, libGLESv2, libxml2, libfreetype, libz, libsecret |
| `sober_services` | 1.1 MB | C++ | GUI services app. Uses libadwaita (GTK4) + WebKitGTK for login/authentication flow. Handles cookie management, auth tokens, Roblox URI scheme handling |
| `libloader.so` | 2.8 MB | Rust | Process spawner and environment setup. Forks/execs the Roblox process, sets up chroot/sandbox, loads libbadcpu.so via LD_PRELOAD style injection, handles /proc/self/maps |
| `libbadcpu.so` | 376 KB | Rust | x86 64 CPU feature emulation. Catches SIGILL (illegal instruction) for missing CPU features (AVX, etc) and emulates them in software. Acts as a signal handler based JIT translator |

### How It Works (High Level)

1. **sober_services** launches first. Its a C++ GTK4/libadwaita app with a WebKit webview that shows the Roblox login page. It handles OAuth authentication, stores cookies via libsoup, and passes auth tokens to the main `sober` binary via IPC
2. **sober** is the core runtime. Written in Rust, it translates the Android ARM64 Roblox binary to run natively on x86 64 Linux. It uses:
   - libEGL/libGLESv2 for GPU rendering (translating Android OpenGL ES calls to desktop Vulkan/OpenGL)
   - libcurl for network operations (connecting to Roblox servers, downloading assets)
   - libxml2 for parsing Android manifest/XML resources
   - libfreetype for font rendering
   - libsecret for secure credential storage
   - libcrypto (OpenSSL) for TLS/crypto operations
   - Custom Android binary translation layer (similar to how Android's ART runtime works, but translating Dalvik/ARM native code to x86 64)
3. **libloader.so** is the process loader. It:
   - Forks and sets up the execution environment (chroot, setuid/gid, process groups)
   - Reads /proc/self/maps to understand memory layout
   - Loads the translated binary into memory with mmap
   - Sets memory protections with mprotect
   - Injects libbadcpu.so into the target process
   - Handles the Android runtime initialization
4. **libbadcpu.so** is a CPU feature emulator. It:
   - Installs SIGILL signal handler
   - Detects CPU features at runtime (SSE4.1, SSSE3, POPCNT, AVX, etc)
   - When the Roblox binary hits an instruction the host CPU does not support, the signal handler catches it
   - Decodes the illegal instruction and emulates it in software
   - Returns execution to the next instruction
   - Shows error dialogs for truly unsupported CPUs (needs at least SSE4.1)

### Key Findings from Reverse Engineering

- **No telemetry or spyware**: The privacy notice accurately describes their data collection. The binary does not contain hidden tracking, keyloggers, or data exfiltration beyond what is documented
- **No Roblox IP redistribution**: Sober downloads the Roblox APK from Google Play on the users behalf. It does not redistribute Roblox code
- **libstanpreg.so**: References to this shared object in libloader suggest it patches or replaces a standard Android library
- **Android translation layer**: The main `sober` binary contains what appears to be a full Android compatibility layer, translating ARM native code, Android syscalls, and Android framework APIs to Linux equivalents
- **mimalloc**: Used as a custom memory allocator (performance optimized)
- **detex library**: Used for texture decompression (Android texture format handling)
- **volk library**: Used as a Vulkan loader (dynamic loading of Vulkan functions)
- **imgui**: Used for in game overlay UI
- **dyncall**: Used for dynamic calling conventions (translating ARM calling conventions to x86 64)
- **mcl/oaknut**: ARM64 assembler/emitter library by merryhime. Used for generating or translating ARM64 instructions
- **nlohmann/json**: JSON parser used in sober_services for configuration and IPC
- **fmt**: C++ formatting library used in sober_services

## Third Party Licenses

Sober includes/attribution for these open source libraries:
- **mimalloc** (MIT) Microsoft Corporation / Daan Leijen
- **detex** (ISC) Harm Hanemaaijer
- **nlohmann/json** (MIT) Niels Lohmann
- **volk** (MIT) Arseny Kapoulkine
- **fmt** (BSD) Victor Zverovich
- **mcl/oaknut** (MIT) merryhime
- **imgui** (MIT) Omar Cornut
- **dyncall** (ISC) Daniel Adler / Tassilo Philipp
- **libxml2** (MIT) Daniel Veillard
- **Android Open Source Project** (Apache 2.0) ChristopherHX / MCMrARM portions

## Implemented Components

### libbadcpu.so - x86-64 CPU Feature Emulator (Issue #8)

Clean room reimplementation of Sober's CPU feature emulator. This shared library:

1. Installs a `SIGILL` signal handler at load time (`__attribute__((constructor))`)
2. Queries CPU features via `CPUID` at startup
3. When the host program hits an instruction the CPU doesn't support, the handler catches `SIGILL`, decodes the faulting instruction, emulates it in software, and modifies `ucontext_t` registers to advance RIP past the instruction
4. Emits a fatal error for unrecognized instructions or unsupported CPUs (requires SSE4.1 minimum)

**Supported emulated instructions:** POPCNT, MOVBE, LZCNT, TZCNT, ANDN, BLSI, BLSMSK, BLSR

## Repository Structure

```
sober-oss/
  README.md                    This file
  ARCHITECTURE.md              Detailed architecture documentation
  SECURITY_AUDIT.md            Security analysis findings
  BINARY_ANALYSIS.md           Per binary analysis notes
  LICENSE                      Project license (MIT)
  meson.build                  Build system (top-level)
  include/
    badcpu.h                   Public API header for libbadcpu
  src/
    libbadcpu/
      badcpu.cpp               Library entry point
      cpuid.cpp                CPUID feature detection
      decoder.cpp              x86-64 instruction decoder
      emulator.cpp             Per-instruction emulation logic
      signal_handler.cpp       SIGILL handler installation/management
      test/
        test_badcpu.cpp        Unit tests
      meson.build              Build definition
  decompiled/
    sober/                    Ghidra decompilation of main runtime
    sober_services/           Ghidra decompilation of GUI services
    libloader/                Ghidra decompilation of process loader
    libbadcpu/                Ghidra decompilation of CPU emulator
  analysis/
    libbadcpu_decompilation.md  Detailed libbadcpu analysis report
    libloader_decompilation.md  Detailed libloader analysis report
    sober_services_decompilation.md  Detailed sober_services analysis report
    *_strings_sorted.txt        Per-binary sorted strings
    imports_*.txt              Per-binary import analysis
  docs/
    config_schema.md           Sober config.json schema documentation
  scripts/
    extract_strings.sh          Binary string extraction
    ghidra_decompile.py        Automated Ghidra headless decompile
```

## Legal Notice

This reverse engineering was performed under the fair use / interoperability provisions of applicable law. The Sober license agreement (notice.txt) prohibits decompilation, but those terms may be unenforceable where interoperability reverse engineering is legally protected.

The decompiled code in this repository is pseudocode produced by Ghidra, not the original source code. It is provided for educational and security research purposes only.

The `src/` directory contains clean room implementations written from scratch based on the documented architecture and analysis. No decompiled pseudocode was copied into the implementation.

## Building from Source

### Prerequisites

- Meson build system (`pip install meson`)
- Ninja build (`apt install ninja-build`)
- GCC or Clang with C++20 support
- Linux x86-64 (uses `cpuid.h` and `ucontext.h`)

### Building libbadcpu.so (Issue #8)

```bash
meson setup build
ninja -C build
```

This produces `build/src/libbadcpu/libbadcpu.so`.

### Running Tests

```bash
meson test -C build
```

### Installing

```bash
meson install -C build --destdir=/path/to/install
```

## Original Project

- Website: https://sober.vinegarhq.org
- Troubleshooting: https://vinegarhq.org/Sober/Troubleshooting.html
- FAQ: https://vinegarhq.org/Sober/FAQ/index.html
- Issues: https://github.com/vinegarhq/sober/issues
- Flatpak: https://flathub.org/en/apps/org.vinegarhq.Sober