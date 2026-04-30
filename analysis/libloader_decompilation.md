# Issue #7: libloader.so Decompilation Report

## Binary Info
- **File**: `libloader.so`
- **Size**: 2.8 MB
- **Language**: Rust (confirmed by panic strings, refcounting patterns, thread infrastructure)
- **Format**: ELF 64-bit LSB shared object, x86-64, stripped
- **Ghidra Coverage**: ~101 functions, ~40K lines of decompiled C pseudocode
- **RPATH**: `$ORIGIN` (loads libbadcpu.so from same directory)

## Architecture Overview

libloader.so is a custom ELF loader and process spawner written in Rust. It is injected into the target process or loaded by the main `sober` binary to set up and launch the Roblox Android runtime.

## Entry Point & Initialization

Rust shared object initialization sequence:
- `_DT_INIT @ 001168a8`: Standard ELF init
- `_INIT_0 / _INIT_1 / _INIT_2`: Rust runtime setup, thread-local storage
- **Runtime startup**: `FUN_00141060` / `FUN_001411b0` - Rust `std::rt::lang_start` with panic handling
- **Constructor functions**: Multiple `FUN_00117xxx` functions that set up the loader state before `main()` equivalent runs

## Key Components

### 1. Process Spawning

**System calls used**: `fork`, `execvp`, `posix_spawnp`, `pidfd_spawnp`, `socketpair`, `pipe2`

libloader implements multiple process creation strategies:
- **Legacy fork/exec**: `fork()` + `execvp()` for basic child process creation
- **Modern posix_spawn**: `posix_spawnp()` with file actions (`posix_spawn_file_actions_addchdir_np`, `adddup2`) for controlled redirection
- **pidfd_spawn**: `pidfd_spawnp()` for Linux-specific pidfd-based child monitoring

**Sandbox setup** (executed in child before exec):
- `chroot`: Filesystem isolation
- `setuid` / `setgid` / `setgroups`: Privilege dropping
- `setpgid` / `setsid`: Session/process group management
- `getrlimit` / `setrlimit`: Resource limit enforcement

### 2. Memory Mapping

**System calls used**: `mmap64`, `mprotect`, `munmap`, `posix_memalign`

Implements an ELF segment loader:
- Reads target binary headers (Roblox APK native libraries)
- `mmap64` code and data segments with appropriate protections
- `mprotect` to adjust RWX permissions as needed
- Anonymous mmap for BSS / heap allocation
- Memory layout is read from `/proc/self/maps` for verification

**Error strings found**:
```
mmap anonymous failed
mmap failed
munmap failed
mprotect failed
```

### 3. Dynamic Library Loading

**API used**: `dlopen`, `dladdr`, `dl_iterate_phdr`

- Loads `libbadcpu.so` from the same directory (RPATH `$ORIGIN`)
- Resolves symbols in the loaded Android binary
- Can inject additional shared objects into the child process

### 4. Network Stack

**System calls used**: `socket`, `socketpair`, `listen`, `accept4`, `connect`, `getsockname`, `getsockopt`

Implements IPC and networking:
- `socketpair` for parent-child IPC (likely primary communication channel with `sober`)
- `socket` + `connect` for network access
- Uses `getaddrinfo` / `freeaddrinfo` / `getifaddrs` for address resolution
- `__res_init` for DNS initialization

### 5. File Operations

**System calls used**: `open64`, `read`, `write`, `writev`, `close`, `lseek64`, `fstat64`, `stat64`, `statx`, `mkdir`, `opendir`, `readdir`, `readdir64`, `rewinddir`

- File I/O for reading APK contents and writing logs
- Directory traversal for finding native libraries inside the APK
- `realpath` for resolving absolute paths
- `readlink` for symlink resolution

### 6. Signal Handling

**System calls used**: `signal`, `sigemptyset`, `sigaddset`, `pthread_sigmask`, `pthread_kill`

- Sets up signal masks in child processes
- Can forward signals between parent and child

## Notable Strings

### Process Paths
```
/app/bin/sober_services    # Path to the GUI service binary
libbadcpu.so               # Injected CPU emulator
/proc/self/maps            # Memory layout introspection
/proc/self/exe             # Path to current binary (must be available)
```

### Easter Egg / Anti-Tamper
```
"you tread on a path I made for you. a path to bring you to Me."
"libstanpreg.so is real."
```
These suggest anti-debugging or anti-tamper measures may be present.

### Error Messages
```
Couldn't read /proc/self/maps
Couldn't open /proc/self/maps
invalid map entry
no /proc/self/exe available. Is /proc mounted?
A previous critical error has already initiated the GUI panic handler. Terminating immediately.
pidfd_spawnp succeeded but the child's PID could not be obtained
nul byte found in provided data
```

## Translation Layer Behavior (Inferred)

libloader is the **process bootstrap layer** of Sober:

1. **Setup phase** (parent process):
   - Reads the Roblox APK manifest
   - Identifies native libraries to load
   - Prepares environment variables and filesystem mappings
   - Creates IPC channels (socketpair or pipe)

2. **Spawn phase**:
   - Forks or posix_spawn's a child process
   - Applies sandbox restrictions (chroot, uid drop)
   - Injects libbadcpu.so into the child
   - Either execve's into the Android runtime OR transfers execution to the mapped binary

3. **Monitor phase**:
   - Reads child stdout/stderr via pipes
   - Handles parent-child IPC via socketpair
   - If child crashes, reports to the GUI panic handler (`sober_services`)

## Relationship to Other Binaries

```
 sober (main)          libloader.so (this file)
     |                           |
     |--fork/exec-->   child process
     |                        |
     |<--socketpair-->   mapped Android runtime
     |                        |
     |                libbadcpu.so (SIGILL emulator)
```

- libloader.so is linked by `sober` (main Rust binary) via `dlopen` or static loading
- libloader.so loads and injects `libbadcpu.so` into the target process
- libloader.so references `/app/bin/sober_services` for the GUI process

## Static Analysis Limitations

- Rust `alloc::sync::Arc` refcounting dominates the code, making ownership tracking difficult
- `std::io::Error` and `std::result::Result<T,E>` are inlined everywhere
- Many functions are generic Rust std patterns (drop glue, clone, etc.) not domain-specific
- The actual ELF parsing logic is deeply nested in `match` statements that Ghidra flattens poorly
- Thread-local storage (`__tls_get_addr`) obfuscates global state access

## Reimplementation Notes

To reimplement libloader:
1. Parse APK/ZIP file format to find `lib/arm64-v8a/` native libraries
2. Read ELF headers and program headers from each .so
3. `mmap` segments at correct virtual addresses with correct protections
4. Perform relocations (REL, RELA, PLT/GOT)
5. Resolve symbols from `DT_NEEDED` entries
6. Create `socketpair(AF_UNIX, SOCK_STREAM)` for parent-child IPC
7. `fork()` then in child:
   - `chroot` to APK extraction dir or bind-mount overlay
   - `setuid`/`setgid` to drop privileges
   - `execve` or jump to mapped ELF entry point
8. In parent: monitor child via pidfd/socket and relay to `sober`

---
*Report generated from Ghidra 11.2 decompilation of libloader.so (2.8MB, stripped Rust ELF)*
