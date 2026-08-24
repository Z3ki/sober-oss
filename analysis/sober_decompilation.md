# Issue #5 / #1: `sober` Main Runtime Decompilation Report

## Binary Info
- **File**: `sober`
- **Size**: 7.1 MB
- **Language**: Rust (confirmed by panic/unwind infrastructure, `__rust_*` allocator shims, TLS-heavy layout) with a large amount of statically linked C/C++ (OpenSSL, libcurl, FreeType, mimalloc)
- **Format**: ELF 64-bit LSB PIE executable, x86-64, stripped
- **Ghidra Coverage**: poor. See "Static Analysis Reality Check" below.

This is the fourth and largest of Sober's binaries and, until now, the only one without a dedicated analysis report (`libbadcpu`, `libloader`, and `sober_services` each have one). It is the process that actually hosts and executes the translated Roblox Android code. Everything the other three binaries do exists to get control into this one.

## Static Analysis Reality Check

Unlike `sober_services` (clean C++, ~843 recovered functions), the main `sober` binary does **not** decompile cleanly. `decompiled/sober/sober.c` is dominated by Ghidra `WARNING: Bad instruction data` / `halt_baddata()` markers starting at the entry point. Three things cause this:

1. **Rust + fat LTO + `mimalloc`**: heavy inlining and a custom global allocator erase the normal function boundaries Ghidra keys on.
2. **Stripped PIE with mixed Rust/C++/C link units**: the symbol table is gone and the code section interleaves Rust std, OpenSSL, curl, and FreeType with no delimiters.
3. **Runtime code generation**: this binary emits and runs code at runtime (see "Memory + Code Generation"), so a large amount of what executes is never present as static instructions for Ghidra to see at all.

Because of this, the honest, defensible findings for `sober` come from its **dynamic symbol imports** and its **embedded string/manifest data**, not from recovered control flow. This report is built from `analysis/imports_sober.txt` and `analysis/sober_strings_sorted.txt`. Every symbol and string quoted below is present in those committed files. Where something is inferred rather than directly observed, it is labelled INFERRED.

## Confirmed Dynamic Dependencies

From `analysis/imports_sober.txt` (`DT_NEEDED`):

```
libloader.so        our own process loader (issue #7)
libmimalloc.so      custom allocator
libcrypto.so.3      OpenSSL 3.x (TLS + PKI)
libcurl.so.4        HTTP/HTTPS client
libfreetype.so.6    font rasterization
libz.so.1           zlib (asset/APK inflate)
libsecret-1.so.0    OS keyring credential storage
libglib-2.0.so.0    glib event loop / utilities
libxml2.so.16       XML parsing (Android manifest/resources)
libGLESv2.so.2      OpenGL ES 2.x rendering
libEGL.so.1         EGL context/surface management
libm / libgcc_s / libc / ld-linux
```

Note what is **not** a hard dependency: Vulkan, X11, Wayland, ALSA, PipeWire, PulseAudio, and the higher GLES variants are all resolved lazily at runtime via `dlopen`, driven by the manifest in the next section.

## Subsystem 1: Platform Abstraction Layer (SDL3-style dynamic backends)

The most useful new discovery. `sober` embeds a set of JSON feature descriptors that drive optional-subsystem loading. They use the exact shape SDL3 uses for its dynamically loaded drivers (`feature` / `description` / `priority` / `soname`). Full set as found in `analysis/sober_strings_sorted.txt`:

| Feature | Priority | soname(s) |
|---|---|---|
| `core-libdbus` | recommended | `libdbus-1.so.3` |
| `audio-libalsa` | suggested | `libasound.so.2` |
| `audio-libpipewire` | suggested | `libpipewire-0.3.so.0` |
| `audio-libpulseaudio` | suggested | `libpulse.so.0` |
| `camera-libpipewire` | suggested | `libpipewire-0.3.so.0` |
| `egl-egl` | suggested | `libEGL.so.1` |
| `egl-es2` | suggested | `libGLESv2.so.2` |
| `egl-es-pvr` | suggested | `libGLES_CM.so.1` |
| `egl-ogl-es` | suggested | `libGLESv1_CM.so.1` |
| `egl-opengl` | suggested | `libGL.so.1`, `libOpenGL.so.0` |
| `wayland` | suggested | `libwayland-client.so.0`, `libwayland-cursor.so.0`, `libwayland-egl.so.1`, `libdecor-0.so.0`, `libxkbcommon.so.0` |
| `wayland-vulkan` | suggested | `libvulkan.so.1` |
| `x11` | recommended | `libX11.so.6`, `libXext.so.6`, `libXcursor.so.1`, `libXfixes.so.3`, `libXi.so.6`, `libXrandr.so.2`, `libXss.so.1`, `libXtst.so.6` |
| `x11-vulkan` | suggested | `libX11-xcb.so.1`, `libvulkan.so.1` |
| `events-udev` | recommended | `libudev.so.1`, `libudev.so.0` |
| `libusb` / `hidabi-libusb` | suggested | `libusb-1.0.so.0` |
| `fribidi` | suggested | `libfribidi.so.0` |
| `Thai` | suggested | `libthai.so.0` |

What this tells us:

- Windowing, input, audio, and GPU-surface plumbing go through an **SDL3 (or SDL3-derived) platform layer**, not hand-rolled per-backend code. This was never documented in `README.md` or `ARCHITECTURE.md`, which only mention EGL/GLESv2 and volk.
- Both **Wayland and X11** are real, with X11 marked `recommended` (the more-tested default) and Wayland `suggested`.
- Audio has three interchangeable backends (**ALSA, PipeWire, PulseAudio**); camera is **PipeWire only**. This matches the `sober_services` settings toggles for audio/camera backend.
- Controller/HID input comes in through **libusb + udev**, i.e. SDL's gamepad stack, not Android's input system.
- `fribidi` + `libthai` mean the runtime does its own **bidirectional / complex-script text shaping** on top of FreeType rather than relying on Android's text stack.

## Subsystem 2: Rendering (EGL + OpenGL ES 2, Vulkan optional)

Confirmed GLES2 symbols in `sober_strings_sorted.txt` include `glGetShaderPrecisionFormat` (GLES-specific), `glGetShaderInfoLog`, `glGetProgramInfoLog`, `glGetActiveUniform`, `glGetActiveAttrib`, `glGetAttribLocation`, `glGetUniformLocation`, `glGetString`, `glGetIntegerv`, `glGetFloatv`, plus `eglGetError`.

So the concrete rendering path is: Android GLES2 calls -> host EGL context (`libEGL.so.1`) -> host `libGLESv2.so.2`. Vulkan is present only through the manifest sonames above and is loaded on demand, consistent with the README's note about `volk` as a dynamic Vulkan loader. The shader-log and precision-format queries show real GLSL ES shader compilation happening at runtime rather than precompiled pipelines.

## Subsystem 3: Networking (libcurl over OpenSSL)

Confirmed: `curl_easy_init`, `curl_easy_setopt`, `curl_easy_perform`, `curl_easy_cleanup`, `curl_easy_strerror`, `curl_slist_append`, `curl_slist_free_all`, versioned against `CURL_OPENSSL_4`. This is a classic single-transfer libcurl usage pattern (init -> setopt -> perform -> cleanup) with custom header lists via `curl_slist`. Used for Roblox API traffic and asset/APK download.

## Subsystem 4: Runtime PKI / local certificate generation (new finding)

`sober` links OpenSSL for more than TLS client work. The symbol set shows it **mints X.509 certificates at runtime**:

```
EVP_PKEY_keygen_init / EVP_PKEY_keygen           generate a key pair
EVP_PKEY_CTX_new_id / EVP_PKEY_CTX_set_ec_paramgen_curve_nid   EC key by curve
EVP_sha256                                       SHA-256 signing digest
X509_new / X509_set_version / X509_set_pubkey
X509_set_issuer_name / X509_set_subject_name
X509_getm_notBefore / X509_getm_notAfter / X509_gmtime_adj    validity window
X509_get_serialNumber / ASN1_INTEGER_set
X509_EXTENSION_create_by_OBJ / X509_add_ext      v3 extensions
X509_sign                                        self-sign
```

Generating an EC key, building a full X.509 cert with v3 extensions and a validity window, and self-signing it with SHA-256 is not something a plain HTTPS *client* needs. Combined with Subsystem 5 (a listening socket), the most likely explanation is that `sober` stands up a **local TLS endpoint on loopback with a self-signed certificate** so the translated Roblox client can talk to a service the runtime hosts in-process (INFERRED). This is worth confirming dynamically and is called out in "Open Questions."

## Subsystem 5: Local sockets and IPC

Confirmed: `socket`, `socketpair`, `bind`/`listen`/`accept` (`listen`, `accept` present), `getaddrinfo`, and the full `inet_*` family (`inet_pton`, `inet_ntop`, `inet_addr`, `inet_aton`, `inet_ntoa`, `inet_network`). `socketpair` is the parent/child channel to `libloader.so` and `sober_services` (matches the `sober_services` finding "Execute Roblox command is bigger than our IPC buffer!"). The `listen`/`accept` pair plus the cert-generation cluster above is the evidence for a real server socket, not just outbound curl.

## Subsystem 6: Credential storage (libsecret)

Confirmed API-level, not just linkage: `secret_password_store_sync` and `secret_password_lookup_sync`. This is the concrete backing for the `use_libsecret` config option documented in `docs/config_schema.md`. Auth material is stored in and read back from the OS keyring synchronously.

## Subsystem 7: Text, fonts, and resource XML

- **FreeType**: `FT_New_Library` + `FT_Add_Default_Modules` (manual library init rather than `FT_Init_FreeType`), `FT_New_Memory_Face` (fonts loaded from memory buffers, i.e. out of the APK, not files), `FT_Load_Glyph`, `FT_Render_Glyph`, `FT_Get_Char_Index`, `FT_Select_Charmap`, `FT_Request_Size`, plus synthetic styling `FT_GlyphSlot_Embolden` / `FT_GlyphSlot_Oblique`.
- **Shaping**: `fribidi` + `libthai` from the manifest sit in front of FreeType for RTL and complex scripts.
- **libxml2**: `xmlParseFile`, `xmlDocGetRootElement`, `xmlGetProp`, `xmlNodeSetContent`, `xmlSaveFormatFileEnc`, `xmlStrcmp`. Read/modify/write of XML, consistent with parsing (and rewriting) Android manifest/resource XML.

## Subsystem 8: Memory and code generation

Confirmed: `mmap64`, `munmap`, `mprotect`, `mremap`, `madvise`, `prctl`, and `sigaction`. The combination of anonymous `mmap` + `mprotect` (to flip pages executable) + `mremap` + `madvise` is the signature of a **JIT / code cache**: this is where the ARM64-to-x86-64 translation output gets written and made runnable. `sigaction` here is the host side of the `libbadcpu.so` contract: `sober` installs the process signal disposition and `libbadcpu` handles `SIGILL` for CPU features the host lacks (see `analysis/libbadcpu_decompilation.md`). `prctl` is likely `PR_SET_NAME` and/or `PR_SET_DUMPABLE`/seccomp setup (INFERRED).

## Subsystem 9: Threading

Full pthread surface: `pthread_create`, `pthread_kill`, condition variables (`pthread_cond_*`), and attribute control (`pthread_attr_setstacksize`, `pthread_attr_setdetachstate`, etc.). `pthread_kill` is how per-thread `SIGILL` delivery stays scoped to the faulting translation thread.

## Subsystem 10: Allocator

`libmimalloc.so` with rpath `$ORIGIN/subprojects/mimalloc:$ORIGIN` (from strings) and `mi_process_info`. mimalloc is the process-wide allocator; the `subprojects/mimalloc` rpath confirms a Meson subproject build layout, same family of build system this OSS repo uses.

## The Android Translation Core (Issue #5): what static analysis can and cannot show

Issue #5 asks to map the ARM64 Android -> x86-64 translation layer. Being honest about the evidence:

**What is supported by the committed artifacts:**
- `libloader.so` extracts `lib/arm64-v8a/*.so` from the APK, mmaps the ELF segments, relocates, and hands control to `sober` (see `analysis/libloader_decompilation.md`). So the input to the translator is real ARM64 Android `.so` code.
- `sober` has the memory + codegen syscall surface (Subsystem 8) needed for a JIT translator and the `SIGILL` path (with `libbadcpu`) for instructions the host cannot run natively.
- The README's third-party attribution list (`mcl/oaknut` ARM64 emitter, `dyncall`, `detex`, `volk`, `dear imgui`) is consistent with a dynamic binary translator: `oaknut`/`mcl` emit host code, `dyncall` bridges calling conventions, `detex` decodes Android/mobile texture formats, `volk` loads Vulkan.

**What is NOT supported by the committed artifacts (do not claim it):**
- No `dalvik`, `libart`, `bionic`, `linker`, `aarch64`, `arm64`, `binder`, or `ashmem` strings survive in `sober_strings_sorted.txt`. The `JNI`-looking hits (`::jNi`, `aJNI`) are sorted-noise fragments, not real symbols. The strings dumps committed here are heavily filtered and alpha-sorted, so absence in them is **not** proof of absence in the binary.
- The actual translation dispatch, block cache structure, and syscall shim table are not recoverable from the current Ghidra output. Claiming specific internals would be fabrication.

**Concrete next steps to actually map the translator (for whoever picks up #5):**
1. Re-run Ghidra on `sober` with a Rust-aware function ID set and a longer analysis timeout; export a fresh `strings sober` with `-n 6` and grep for `arm64-v8a`, `oaknut`, `Xbyak`, `art::`, `bionic`, `__system_property`, `dlopen` of `libc.so`/`liblog.so`.
2. Run the live Flatpak under `strace -f -e trace=memory,signal` and watch for the `mmap(PROT_NONE)` -> `mprotect(PROT_EXEC)` code-cache pattern and `SIGILL` deliveries; dump the mapped executable regions to recover emitted host blocks.
3. `LD_PRELOAD` a shim over `mmap`/`mprotect` to log every page that goes executable, then correlate RIP ranges with `/proc/self/maps` to isolate the translation cache from static code.
4. Break on `dlopen` to enumerate which manifest sonames actually load on a given host (confirms X11-vs-Wayland and audio-backend selection at runtime).

## Open Questions

- Does the runtime `X509`/`listen` pair really stand up a loopback TLS server, and what talks to it (the translated client, `sober_services`, or a Roblox subprocess)? Confirm with `ss -ltnp` while running and a `SSLKEYLOGFILE` capture.
- Is `prctl` used for seccomp sandboxing, thread naming, or both?
- Which single audio/video backend wins on a default GNOME-Wayland vs KDE-X11 host, and is that what the `sober_services` toggles actually change?

## Reproduction

```bash
# imports
readelf -d ./sober | grep NEEDED

# survived strings, wider net than the committed dumps
strings -n 6 ./sober | grep -Ei '"feature"|soname|EVP_PKEY|X509_|glGet|FT_|secret_|curl_easy'

# the SDL3-style backend manifest
strings -n 6 ./sober | grep -o '\[{"feature".*}\]'
```

## Static Analysis Limitations

- Ghidra cannot recover clean function boundaries from this Rust+LTO+mimalloc binary; `decompiled/sober/sober.c` is mostly `halt_baddata` and should be treated as a coverage marker, not source.
- The committed strings dumps are alpha-sorted and filtered, so they lose multi-word context and drop many real symbols. Findings here are the intersection of what those dumps and the import table both support.
- A large fraction of executed code is JIT output that never exists statically.

## Reimplementation Notes

To reimplement the `sober` host runtime:
1. Take the mmapped `arm64-v8a` ELF images from the loader (issue #7).
2. Build an ARM64 -> x86-64 dynamic binary translator with a `mmap`+`mprotect` code cache; use `oaknut`/`mcl` to emit host code and `dyncall` for cross-ABI calls.
3. Install a `SIGILL` handler (or reuse `libbadcpu.so`) for host-missing instructions.
4. Bring up a platform layer via SDL3: create the window/GL surface, wire audio (ALSA/PipeWire/PulseAudio), input (udev/libusb), on Wayland or X11 per the manifest.
5. Render through host EGL + GLES2, loading Vulkan lazily where available.
6. Provide the shims the Android code calls: FreeType (from in-APK font buffers) with fribidi/libthai shaping, libxml2 for resources, zlib for inflate.
7. Networking via libcurl; store credentials via libsecret; generate the local EC/X.509 cert if a loopback TLS service is required.
8. IPC to `sober_services` over the `socketpair` JSON channel (`executeRoblox`).

---
*Report built from `analysis/imports_sober.txt` and `analysis/sober_strings_sorted.txt`. Symbol- and manifest-level analysis; the main `sober` binary does not yield usable Ghidra control-flow, so claims are scoped to what the import table and surviving strings support.*
