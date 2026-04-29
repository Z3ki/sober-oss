# Sober Architecture Deep Dive

## Process Lifecycle

### 1. Service Process (sober_services)

**Language**: C++ (with nlohmann/json, fmt)
**UI Framework**: GTK4 + libadwaita + WebKitGTK

The service process provides:
- Login/authentication UI using WebKitGTK webview
- Roblox OAuth flow through embedded browser
- Cookie management via libsoup (with http_only and secure flags)
- Auth token storage and IPC to main process
- Settings/preferences UI (adw_switch_row, adw_expander_row)
- File picker for APK selection (gtk_file_dialog)
- Error dialogs (adw_alert_dialog)
- Discord join link handling (x scheme handler roblox: and roblox player:)

**Key strings found**:
- `org.vinegarhq.Sober` D Bus/app ID
- `/org/vinegarhq/Sober/onboarding.ui` embedded GTK UI resource
- `RobloxWKHybrid` custom WebKit script message handler
- `executeRoblox` IPC command
- `script-message-received::RobloxWKHybrid` WebKit message channel
- `settings-onboard-page`, `settings-continue-button` UI element IDs
- `config-switch-*`, `config-sH1` settings configuration patterns

### 2. Main Runtime (sober)

**Language**: Rust (compiled with panic handlers, thread infrastructure)
**Binary Size**: 7.1 MB

This is the core Android binary translator. It:

#### Android Compatibility Layer
- Translates Android ARM64 binaries to x86 64
- Maps Android syscalls to Linux equivalents
- Provides Android runtime environment (bionic libc shims, Android framework stubs)
- Handles Android intent system (roblox:// URI schemes)
- Translates Android OpenGL ES calls to desktop Vulkan/OpenGL via volk

#### Rendering Pipeline
- Uses EGL and GLESv2 for OpenGL ES compatibility
- Vulkan backend via volk (dynamic Vulkan loader)
- Supports Wayland and X11 backends
- Texture translation pipeline using detex library
- Font rendering via libfreetype

#### Networking
- HTTP/HTTPS via libcurl (OpenSSL backend)
- Connects to Roblox game servers
- Downloads Android APK bundles from Google Play
- Crash reporting

#### Process Management
- Uses fork/exec for subprocess management
- Installs signal handlers (SIGILL for libbadcpu integration)
- Manages child processes (kill, killpg)
- Resource limits (getrlimit/setrlimit)
- Environment variable management (environ, getenv/putenv/setenv)

#### Memory Management
- Custom allocator: mimalloc (libmimalloc.so)
- Direct mmap/mprotect/munmap calls for memory mapping
- Anonymous mmap for code generation

### 3. Process Loader (libloader.so)

**Language**: Rust
**Binary Size**: 2.8 MB

Responsible for:
- Reading and parsing ELF binary headers
- mmap code and data segments into memory
- Setting up memory protections (r/w/x) via mprotect
- Process forking and environment setup
- chroot: can set up isolated filesystem views
- setuid/setgid/setgroups/setpgid: privilege management
- setsid: session management
- posix_spawnp: process spawning
- Loading additional shared libraries (dlopen)
- Reading /proc/self/maps to understand memory layout
- Injecting libbadcpu.so into the spawned process

**Key string found**: `"you tread on a path I made for you. a path to bring you to Me."` appears to be an easter egg or anti tampering message.

### 4. CPU Feature Emulator (libbadcpu.so)

**Language**: Rust
**Binary Size**: 376 KB

Purpose: Emulate x86 64 CPU instructions that the host CPU does not support (AVX, SSE4.2, etc).

How it works:
1. Installs a SIGILL signal handler
2. Detects available CPU features using CPUID
3. When the Roblox binary executes an instruction the host CPU does not support:
   - The CPU raises SIGILL (Illegal Instruction)
   - The signal handler catches it
   - The handler reads the faulting instruction bytes from the siginfo structure
   - Decodes the instruction (uses internal x86 disassembler)
   - Emulates the instruction behavior in software
   - Modifies the trapped thread context to reflect the result
   - Returns from the signal handler, resuming at the next instruction
4. For CPUs without SSE4.1 (minimum requirement): shows an error dialog explaining the hardware is too old

**CPU feature checks found**:
- SSE4.1 (minimum requirement)
- SSSE3
- POPCNT
- CPU vendor string detection (Intel vs AMD)
- CPU brand string parsing

This is similar in concept to QEMU user mode emulation, but only for specific instructions rather than full emulation.

## Inter Process Communication

1. `sober_services` opens and authenticates via WebKit webview
2. Auth cookies/tokens are stored via WebKit cookie manager and/or libsecret
3. `sober_services` sends IPC message `executeRoblox` to launch the game
4. `sober` main process receives the launch command with auth info
5. `sober` uses `libloader.so` to set up the Roblox Android binary execution environment
6. `libloader.so` forks, sets up sandbox (chroot, privs), mmaps the binary, injects `libbadcpu.so`
7. `sober` runtime begins translating and executing the Roblox Android binary
8. `libbadcpu.so` handles any illegal instruction traps during execution

## Configuration System

Sober reads a `config.json` file (referenced in strings). Known config options from release notes:
- `use_libsecret`: Store auth cookies securely via libsecret
- Server location indicators
- Texture customization
- Quality level setting
- Discord activity/RPC integration
- Close on experience leave
- WebView enable/disable

## Security Model

- Flatpak sandboxing with minimal permissions (DRI, network, Wayland/X11, PulseAudio)
- `--allow=devel` required for ptrace access (needed by libbadcpu.so for signal handling)
- Telemetry and ads are disabled by default
- libsecret integration for secure credential storage
- HTTP only cookies set to secure when stored
- No root privileges required beyond flat sandbox permissions