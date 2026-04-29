# Binary Analysis: Per Binary Notes

## sober (Main Runtime)

**File**: `sober`
**Size**: 7.1 MB
**Format**: ELF 64 bit LSB PIE executable, x86 64
**Language**: Rust (confirmed by panic handler strings, Rust error types, thread naming)
**Stripped**: Yes (no section headers, no debug symbols)
**Build ID**: 4326c9c9a3e8c7df93715a360e853fe4c61d384a
**RELRO**: Full
**Stack Canary**: No
**NX**: Yes
**PIE**: Yes

### Dynamic Dependencies
```
libloader.so (RPATH: $ORIGIN/subprojects/mimalloc:$ORIGIN)
libmimalloc.so
libcrypto.so.3 (OpenSSL)
libcurl.so.4
libfreetype.so.6
libz.so.1
libsecret-1.so.0
libglib-2.0.so.0
libxml2.so.16
libGLESv2.so.2
libEGL.so.1
libm.so.6
libgcc_s.so.1
libc.so.6
ld linux x86 64.so.2
```

### Key Strings
- SDL3 feature descriptions (audio, video, input subsystems)
- Vulkan/OpenGL rendering strings
- glibc version info: `{"type":"deb","os":"ubuntu","name":"glibc","version":"2.42-0ubuntu3.1","architecture":"amd64"}`
- Android compatibility strings (JNI, Dalvik related patterns)

### Embedded Libraries (statically linked)
- detex (texture decompression)
- volk (Vulkan loader)
- imgui (immediate mode GUI)
- dyncall (dynamic calling convention)
- mcl/oaknut (ARM64 assembler)
- SDL3 subsystems (audio, video, input, haptic, gamepad)

---

## sober_services (GUI Authentication)

**File**: `sober_services`
**Size**: 1.1 MB
**Format**: ELF 64 bit LSB PIE executable, x86 64
**Language**: C++ (confirmed by C++ RTTI, nlohmann/json symbols, fmt symbols)
**Stripped**: Yes (no section headers)
**RELRO**: Full
**NX**: Yes
**PIE**: Yes

### Dynamic Dependencies
```
libwebkitgtk-6.0.so.4
libgtk-4.so.1
libsoup-3.0.so.0
libglib-2.0.so.0
libgio-2.0.so.0
libjavascriptcoregtk-6.0.so.1
libgobject-2.0.so.0
libadwaita-1.so.0
libgcc_s.so.1
libc.so.6
ld-linux-x86-64.so.2
```

### Key Strings
- `/org/vinegarhq/Sober/onboarding.ui` (embedded GTK UI resource path)
- `org.vinegarhq.Sober` (app ID)
- `RobloxWKHybrid` (WebKit message handler name)
- `executeRoblox` (IPC command)
- `script-message-received::RobloxWKHybrid` (WebKit signal)
- `settings-onboard-page`, `settings-continue-button` (UI element IDs)
- `config-switch-*`, `config-sH1` (settings patterns)
- `navigation-view` (AdwNavigationView)
- `Autht` (authentication string fragment)
- `Auth` (authentication string)
- `random_device::random_device(const std::string&): unsupported token` (C++ std::random_device error)
- nlohmann/json type names: `type_error`, `parse_error`
- WebKit cookie operations: `soup_cookie_new`, `soup_cookie_set_secure`, `soup_cookie_set_http_only`

### UI Architecture
- AdwApplication (libadwaita based)
- NavigationView for page flow (onboarding to settings)
- WebKitWebView for Roblox login
- AlertDialog for error messages
- SwitchRow and ExpanderRow for settings
- FileDialog for APK selection
- GTK Builder for UI templates (loads from embedded resources)

---

## libloader.so (Process Loader)

**File**: `libloader.so`
**Size**: 2.8 MB
**Format**: ELF 64 bit LSB shared object, x86 64
**Language**: Rust (confirmed by panic handler strings, error types)
**Stripped**: Yes (no section headers)
**RPATH**: $ORIGIN (loads libbadcpu.so from same directory)
**RELRO**: Regular (not full)
**NX**: Yes

### Dynamic Dependencies
```
libbadcpu.so
libgcc_s.so.1
libc.so.6
ld-linux-x86-64.so.2
```

### Key Strings
- `"you tread on a path I made for you. a path to bring you to Me."` (easter egg / anti tamper)
- `"libstanpreg.so is real."` (internal library reference)
- `/proc/self/maps` (memory layout reading)
- `/app/bin/sober_services` (service binary path)
- Fork/exec patterns: fork, execvp, posix_spawnp, pidfd_spawnp
- Sandbox: chroot, setuid, setgid, setgroups, setpgid, setsid
- Memory: mmap, mprotect, munmap, posix_memalign
- File: read, write, open, stat, mkdir, opendir, readdir
- Network: socket, listen, accept4, connect
- Signals: signal, sigaction, sigemptyset, sigaddset, pthread_kill, pthread_sigmask
- Dynamic loading: dlopen

### Purpose
This is a custom ELF loader and process spawner. It:
1. Reads the target binary (Roblox APK native code)
2. mmaps code and data segments into memory
3. Sets memory protections (rwx as needed)
4. Forks a new process
5. Sets up sandboxing (chroot, uid/gid dropping)
6. Injects libbadcpu.so into the child process
7. Transfers execution to the loaded binary
8. Monitors the child process

---

## libbadcpu.so (CPU Emulator)

**File**: `libbadcpu.so`
**Size**: 376 KB
**Format**: ELF 64 bit LSB shared object, x86 64
**Language**: Rust (confirmed by panic strings, thread infrastructure)
**Stripped**: Yes (no section headers)
**RELRO**: Regular
**NX**: Yes

### Dynamic Dependencies
```
libgcc_s.so.1
libc.so.6
ld-linux-x86-64.so.2
```

### Key Strings
- `(Illegal Instruction)` (SIGILL description)
- `CPU Brand:` / `CPU Vendor:` / `CPU Feature Support:` (CPUID detection)
- `Received signal:` (signal handler message)
- `This program requires a CPU with x86-64-v2 features.` (minimum requirement)
- `We attempted to emulate missing features, but encountered an instruction that could not be handled.` (emulation failure)
- `SSSE3:` / `POPCNT:` (CPU feature check labels)
- `FATAL: Unrecognized Instruction` / `Instruction Bytes:` (debug output)
- `Your CPU does not support at least SSE 4.1` (hard fail message)
- `thread panicked while processing panic. aborting.` (Rust double panic)
- `Error setting SIGILL handler` (signal handler install failure)

### Architecture
This is a signal handler based x86 instruction emulator:
1. At load time: installs SIGILL signal handler via sigaction
2. At load time: queries CPU features via CPUID instruction
3. At runtime: when a trapped instruction fires SIGILL:
   a. Signal handler receives siginfo_t with fault address and instruction bytes
   b. Handler decodes the instruction (internal x86 disassembler)
   c. Handler emulates the instruction behavior
   d. Handler modifies ucontext_t to set results and advance RIP past the instruction
   e. Handler returns, execution continues
4. If the CPU lacks SSE4.1 entirely: shows a GTK error dialog and exits