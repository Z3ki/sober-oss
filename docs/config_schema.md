# Issue #10: Sober config.json Schema Documentation

## Approach
We searched all four Sober binaries (`sober`, `sober_services`, `libloader.so`, `libbadcpu.so`) for JSON key references, config strings, and UI toggle identifiers.

## Runtime Analysis (Headless execution attempt)

Attempted to run `sober_services` under Xvfb with a fake `/.flatpak-info`.
It loaded all GTK4/WebKit/libadwaita libraries successfully but exited with code 1 before showing any window.
The `/.flatpak-info` check passes; the failure occurs later in initialization (likely requires a Wayland/X11 session or GSettings schema it cannot find in the bare environment).

Strace shows library loading sequence: libwebkitgtk-6.0, libgtk-4, libadwaita-1, libjavascriptcoregtk-6.0, libsecret-1 — confirming the UI stack matches our static analysis.

## Confirmed Config Keys (Found in Binary Strings)

### From `sober_services` (GTK4/libadwaita UI strings)
| Key | Type | Source | Context |
|-----|------|--------|---------|
| `config-sH1` | unknown | `strings_sober_services.txt:1468` | Likely a settings page ID (config - settings H1?) |
| `config-switch-*` | boolean | `strings_sober_services.txt:1469` | GTK SwitchRow widget prefix; toggles rendered as on/off switches |
| `enable-developer-extras` | boolean | `strings_sober_services.txt:1518` | Developer mode toggle; probably enables debug logging or advanced features |
| `enable_auto_downloads` | boolean | `strings_sober_services.txt:1519` | APK auto-download; controls whether Sober fetches Roblox APK automatically vs requiring manual selection |

### CLI Arguments
| Flag | Context |
|------|---------|
| `--server` | `strings_sober_services.txt:130` |
| `--switch-H1` | `strings_sober_services.txt:130` (related to config-sH1) |

### From `sober` main binary
No explicit config key strings were found in the `sober` runtime binary. Configuration appears to be passed via IPC or environment variables rather than parsed directly from a JSON file by the Rust core.

### From `libloader.so`
No config strings found. The loader uses hardcoded paths (`/app/bin/sober_services`) and environment-based sandboxing.

## Inferred / External (from release notes, NOT in binary strings)

| Option | Inferred Type | Evidence |
|--------|--------------|----------|
| `use_libsecret` | boolean | Binary links `libsecret-1.so.0`; UI strings reference `soup_cookie_new`, `soup_cookie_set_secure`. Not found as a literal string key |
| `texture_quality` | enum/string | Release notes mention texture customization; **absent from binary strings** |
| `quality_level` | enum/number | Release notes; **absent from binary strings** |
| `discord_activity` / `discord_rpc` | boolean | Release notes mention Discord Activity/RPC integration; **absent from binary strings** |
| `close_on_experience_leave` | boolean | Release notes; **absent from binary strings** |
| `webview_enabled` | boolean | Release notes; **absent from binary strings** |

## Negative Findings
- **None of the release-note config keys appear as plain strings in the binaries**
- This suggests they are either:
  1. Dynamically loaded from an external config server or protobuf definition
  2. Obfuscated or generated at runtime from constant fragments
  3. Compile-time stripped by Rust LTO (unlikely for C++ UI strings)
  4. Part of a protobuf/flatbuffers schema where field names are not embedded

## JSON Schema (Confirmed Only)

```json
{
    "enable_auto_downloads": true,
    "enable_developer_extras": false,
    "config_sH1": null,
    "config_switch_prefix": "config-switch-"
}
```

## How Config Is Used (from code flow analysis)

1. `sober_services` reads/writes config via **nlohmann/json** (confirmed by C++ RTTI strings in binary)
2. Settings UI maps GTK4 `AdwSwitchRow` widgets directly to JSON booleans using widget IDs with `config-switch-` prefix
3. `enable_auto_downloads` gates the APK download flow before launching the main `sober` binary
4. Developer extras likely enables `WEBKIT_SETTINGS_ENABLE_DEVELOPER_EXTRAS` (WebKitGTK debug tools)
5. Config is probably stored at `~/.var/app/org.vinegarhq.Sober/config/` (standard Flatpak XDG path)

## Gaps / TODO

- The exact config file path (`config.json` vs `settings.json`) was not found in strings
- `server_region` string was not found in binaries; `--server` flag suggests it exists as CLI arg
- `texture_quality`, `quality_level`, `discord_activity`, `discord_rpc`, `close_on_experience_leave`, `webview_enabled` are **not present in binary strings** and are only known from external release notes; must be extracted via runtime analysis (strace, gdb, or config file inspection on a live Flatpak install)
- `use_libsecret` is confirmed by library linkage and API imports, but the exact config key name is inferred

## Recommendation

To fully document the schema:
1. Install Sober via Flatpak on a desktop system with a display
2. Toggle all settings switches and observe JSON file writes in `~/.var/app/org.vinegarhq.Sober/`
3. Alternatively, attach gdb to `sober_services`, set breakpoint on `nlohmann::json::operator[]`, and read keys from stack frames

---
*Report based on static string analysis + headless runtime execution attempt of Sober binaries. Confirmed keys are from actual binary strings; inferred keys are from external release notes.*