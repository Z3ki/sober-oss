# Sober Security Audit

**Date**: 2026 04 29
**Version**: 1.6.7 (Roblox 710)
**Auditor**: Maxwell (automated reverse engineering analysis)

## Summary

Sober is a proprietary Roblox on Linux runtime. The security posture is reasonable for a community developed tool. No malicious behavior, spyware, or undisclosed data collection was found.

## Findings

### POSITIVE: No Hidden Telemetry

The binaries do not contain undisclosed telemetry endpoints, tracking pixels, or data exfiltration code beyond what is described in their privacy policy. Network connections are to:
- Cloudflare (CDN for updates)
- Roblox servers (game traffic)
- ipinfo.io (optional server location feature, user opt in)
- Google Play (optional APK download)

### POSITIVE: No Code Obfuscation

While the binaries are stripped (no debug symbols), they are not obfuscated with control flow flattening, string encryption, or packing. Standard Ghidra decompilation works without issues. This suggests the developers prioritize performance over anti reverse engineering.

### POSITIVE: Minimal Sandbox Permissions

The Flatpak permissions are minimal:
- DRI (GPU access, required)
- Network (required for online game)
- Wayland/X11 (display server, required)
- PulseAudio (audio, required)
- IPC (X11 performance, noted as regrettable)
- devel (ptrace, required for SIGILL handler in libbadcpu)

### POSITIVE: Secure Cookie Handling

sober_services properly sets:
- `soup_cookie_set_secure()` marks cookies as secure
- `soup_cookie_set_http_only()` prevents JavaScript access
- Cookies stored via WebKit cookie manager

### NEUTRAL: Anti Reverse Engineering Clause

The notice.txt explicitly prohibits reverse engineering. However:
- The binaries are not technically protected (no DRM, no obfuscation)
- This clause may be unenforceable in jurisdictions with interoperability exceptions
- The stated reason for closed source (preventing abuse) is legitimate

### NEUTRAL: libbadcpu.so Signal Handler

The SIGILL handler approach for CPU emulation is a legitimate technique but:
- Modifies thread execution context at signal level
- Could theoretically be used to hide malicious code injection
- In practice, the implementation is straightforward CPU instruction emulation

### NEUTRAL: libloader.so Process Injection

libloader.so uses fork/exec + mmap + mprotect to load binaries:
- This is a standard binary loader pattern (similar to ld.so)
- It reads /proc/self/maps (could be used for memory inspection)
- It performs chroot, setuid, setgid (privilege management)
- No evidence of malicious injection, only the documented Roblox binary loading

### NOTE: Easter Egg / Anti Tampering Message

libloader.so contains the string `"you tread on a path I made for you. a path to bring you to Me."` which appears to be either an easter egg or an anti tampering message displayed when certain integrity checks fail.

### NOTE: libstanpreg.so Reference

libloader.so references `libstanpreg.so` with the string `"libstanpreg.so is real."` This appears to be an internal library name, possibly related to Android standard regex or signal handling.

### NOTE: Android Open Source Code

The notice.txt attributes code to ChristopherHX and MCMrARM under MIT license, plus Android Open Source Project under Apache 2.0. This suggests portions of the Android compatibility layer are derived from or inspired by open source Android implementations.

## Binary Integrity

| Binary | Strip | Obfuscation | Packer | Notes |
|---|---|---|---|---|
| sober | Full (no section headers) | None | None | Standard Rust strip |
| sober_services | Full (no section headers) | None | None | Standard C++ strip |
| libloader.so | Full (no sections) | None | None | Standard Rust strip |
| libbadcpu.so | Full (no sections) | None | None | Standard Rust strip |
| libmimalloc.so | Full | None | None | Standard mimalloc build |

## Dynamic Dependencies

### sober (main runtime)
- libloader.so (custom, injected)
- libmimalloc.so (custom, memory allocator)
- libcrypto.so.3 (OpenSSL, TLS)
- libcurl.so.4 (HTTP client)
- libfreetype.so.6 (font rendering)
- libz.so.1 (compression)
- libsecret-1.so.0 (secure credential storage)
- libglib-2.0.so.0 (GLib, base)
- libxml2.so.16 (XML parsing)
- libGLESv2.so.2 (OpenGL ES)
- libEGL.so.1 (EGL)
- libm.so.6 (math)
- libgcc_s.so.1 (GCC runtime)
- libc.so.6 (glibc)

### sober_services (GUI)
- libwebkitgtk-6.0.so.4 (WebKit web rendering)
- libgtk-4.so.1 (GTK4 UI)
- libsoup-3.0.so.0 (HTTP, cookief)
- libglib-2.0.so.0, libgio-2.0.so.0, libgobject-2.0.so.0 (GLib)
- libjavascriptcoregtk-6.0.so.1 (JavaScript engine)
- libadwaita-1.so.0 (Adwaita UI)
- libgcc_s.so.1, libc.so.6 (runtime)

### libloader.so (process loader)
- libbadcpu.so (CPU emulator, injected)
- libgcc_s.so.1, libc.so.6 (runtime)
- Uses: fork, exec, chroot, mmap, mprotect, dlopen, /proc/self/maps

### libbadcpu.so (CPU emulator)
- libgcc_s.so.1, libc.so.6 (runtime only)
- Self contained signal handler based emulator

## Recommendations for Users

1. Sober is reasonably secure for what it does. The code is not obfuscated and can be audited
2. The Flatpak sandbox provides good isolation
3. Auth tokens are handled securely (libsecret, http only cookies)
4. Users should be aware of the `--allow=devel` permission (ptrace access)
5. Network traffic appears limited to what is documented