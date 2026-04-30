# Issue #6: sober_services Decompilation Report

## Binary Info
- **File**: `sober_services`
- **Size**: 1.1 MB
- **Language**: C++ (confirmed by C++ RTTI, nlohmann/json symbols, fmt library symbols, vtables)
- **Format**: ELF 64-bit LSB PIE executable, x86-64, stripped
- **Ghidra Coverage**: ~843 functions, ~60K lines of decompiled C pseudocode
- **RELRO**: Full, **NX**: Yes, **PIE**: Yes

## Architecture Overview

sober_services is the GUI authentication and onboarding process. It runs as a separate process from the main `sober` runtime and communicates via IPC. It uses GTK4 + libadwaita + WebKitGTK for the Linux desktop UI and handles the Roblox OAuth login flow.

## Entry Point & Initialization

Standard C++ program structure:
- `entry @ 0013b380`: Calls `__libc_start_main` with `DAT_001faa10` as the actual `main()` function
- `_FINI_0 @ 0013b420`: C++ static destructor teardown (`__cxa_finalize`)
- `_INIT_1 @ 0013b460`: C++ static constructor initialization
- Rust style vtable indirections present due to mixed Rust stdlib linkage in the build environment

## Key Components

### 1. Application Framework (GTK4 + libadwaita)

**Imports** (dynamically linked):
```
libwebkitgtk-6.0.so.4
libgtk-4.so.1
libadwaita-1.so.0
libglib-2.0.so.0 (g_object_ref, g_signal_connect, g_list_model, etc.)
libgio-2.0.so.0 (GApplication, GFile, etc.)
gobject-2.0.so.0 (GObject system)
```

**Key GTK Functions Used**:
- `adw_application_new` - Creates the main AdwApplication instance
- `g_application_run` - Enters GTK main loop
- `adw_application_window_new` / `adw_application_window_set_content` - Main window
- `adw_navigation_view_push_by_tag` / `adw_navigation_view_replace_with_tags` - Page navigation
- `adw_alert_dialog_new` / `adw_alert_dialog_add_response` / `adw_dialog_present` - Error dialogs
- `adw_switch_row_get_active` / `adw_switch_row_set_active` - Toggle settings
- `adw_expander_row_set_enable_expansion` - Expandable settings sections
- `gtk_file_dialog_new` / `gtk_file_dialog_open_multiple` - APK file picker
- `gtk_button_new` / `gtk_label_new` / `gtk_box_new` - Basic widgets
- `gtk_builder_new` / `gtk_builder_add_from_resource` - Loads UI from embedded resources
- `gtk_widget_add_css_class` / `gtk_widget_set_visible` - Styling and visibility
- `g_application_hold` / `g_application_quit` - Lifecycle management
- `g_timeout_add` - Periodic callbacks

### 2. WebKit Integration (Roblox Login)

**Key WebKit Functions**:
- `webkit_web_view_new` - Creates the embedded browser
- `webkit_web_view_load_uri` - Loads Roblox login/cookie URLs
- `webkit_web_view_evaluate_javascript` - Injects JS to extract auth tokens
- `webkit_web_view_evaluate_javascript_finish` - Async completion handler
- `webkit_web_view_get_user_content_manager` - Access to JS message handlers
- `webkit_web_view_get_settings` / `webkit_web_view_set_settings` - Browser config
- `webkit_web_view_get_context` / `webkit_web_view_get_network_session` - Session/cookie management

**Custom JS Message Handlers**:
- `RobloxWKHybrid` - Bidirectional message channel between JS and C++
- `script-message-received::RobloxWKHybrid` - GTK signal connection for incoming messages
- `executeRoblox` - IPC command sent from webview to C++ after successful auth

**Cookie Management**:
- `soup_cookie_new` - Create HTTP cookies
- `soup_cookie_set_secure` / `soup_cookie_set_http_only` - Security flags
- `webkit_cookie_manager_add_cookie` - Store auth cookies in WebKit cookie jar

### 3. Settings / Configuration UI

**UI Element IDs** (from strings):
- `settings-onboard-page` - Main onboarding page
- `settings-continue-button` - Proceed button
- `config-switch-*` - Settings toggle pattern (for various boolean options)
- `config-sH1` - Settings category indicator

**Settings managed** (inferred from strings):
- APK file location
- Audio backend (pipewire)
- Camera (pipewire)
- Graphics renderer (Vulkan/OpenGL)
- Display backend (Wayland/X11)
- Discord RPC integration

### 4. IPC Communication

**IPC Mechanisms** (inferred from strings and Flatpak metadata):
- `executeRoblox` - Command sent from sober_services to main `sober` process to start the game
- D-Bus IPC: `org.vinegarhq.Sober` app ID (Flatpak/D-Bus registration)
- `socketpair` / `pipe2` - Low-level parent-child IPC (from strace and strings analysis)
- "Execute Roblox command is bigger than our IPC buffer!" - Confirms fixed-size IPC buffer between processes

### 5. JSON / Data Processing

**Libraries**: nlohmann/json (C++ JSON library)

**Found in strings**:
- `N8nlohmann16json_abi_v3_11_36detail10type_errorE` - JSON type error
- `N8nlohmann16json_abi_v3_11_36detail11parse_errorE` - JSON parse error
- The binary reads/writes JSON for:
  - Auth token exchange with Roblox API
  - Configuration file (`config.json`)
  - IPC protocol serialization

### 6. File Picker

**Android APK selection flow**:
- Uses `gtk_file_dialog_new` with APK filter (`apkm`, `apks` patterns)
- `gtk_file_filter_add_suffix("apk")` / `gtk_file_filter_add_suffix("apks")`
- Returns selected file path to main `sober` process via IPC

## Notable Strings

### UI Resources
```
/org/vinegarhq/Sober/onboarding.ui
org.vinegarhq.Sober
```

### WebKit Message Channels
```
RobloxWKHybrid
executeRoblox
script-message-received::RobloxWKHybrid
script-message-received::executeRoblox
```

### Error / User Messages
```
Android application bundle files
Execute Roblox command is bigger than our IPC buffer! Please report this!
```

### C++ Library Internals
```
random_device::random_device(const std::string&): unsupported token
description() is deprecated; use Display
```

## Process Architecture

```
sober_services (this binary)
    |
    |-- Main UI Thread --> GTK event loop + AdwApplication
    |    |
    |    |-- WebKitWebView --> loads Roblox login page
    |    |    |
    |    |    |-- JS <-> C++ via RobloxWKHybrid
    |    |
    |    |-- Settings UI --> AdwNavigationView with config toggles
    |    |
    |    |-- File Picker --> GTK4 FileDialog for APK selection
    |
    |-- IPC Thread --> sends executeRoblox command to main sober process
    |
    |-- Error Handler --> AdwAlertDialog for crashes/misconfigurations
```

## Relationship to Other Binaries

- `sober_services` is launched by `sober` (main runtime) as a subprocess or via D-Bus activation
- After successful login, `sober_services` sends `executeRoblox` IPC message back to `sober`
- `sober_services` stores auth cookies in WebKit cookie jar, which `sober` can also access
- `sober_services` references `/app/bin/sober_services` as its own path (from libloader strings)

## Static Analysis Limitations

- Heavy C++ template instantiation (nlohmann/json, fmt, std::string) creates noise in decompilation
- GTK signal callbacks use GObject closures that hide actual handler functions
- WebKit JS evaluation is async (`evaluate_javascript_finish` pattern), making control flow non-linear
- The actual IPC buffer format is not visible in decompilation (optimized into raw memory operations)
- Settings toggle logic is buried in GObject property bindings rather than explicit function calls

## Reimplementation Notes

To reimplement sober_services:
1. Create `AdwApplication` with app ID `org.vinegarhq.Sober`
2. Load UI from embedded GTK Builder XML (`/org/vinegarhq/Sober/onboarding.ui`)
3. Create `AdwNavigationView` with pages: Onboarding, Settings, Login
4. Embed `WebKitWebView` for Roblox login
   - Register `RobloxWKHybrid` script message handler
   - Extract auth tokens from JS via `webkit_web_view_evaluate_javascript`
   - Store cookies with `SoupCookie` + `WebKitCookieManager`
5. Settings page: `AdwSwitchRow` for toggles, `GtkFileDialog` for APK selection
6. IPC: Create Unix socket, send JSON containing `{"command":"executeRoblox","token":"..."}`
7. Error handling: `AdwAlertDialog` for user-facing messages

---
*Report generated from Ghidra 11.2 decompilation of sober_services (1.1MB, stripped C++ ELF)*
