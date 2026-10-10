# Melty for Linux — community port (1.0.76 Fix 10)

An **unofficial Linux adaptation** of the Windows desktop application available from [melty.gg](https://melty.gg). This repository provides the community-written **Linux integration code, guarded patches, build scripts, and installer source**. It does **not** contain the Melty proprietary application or its official installer.

## Features

- Steam library discovery for native Steam and Steam Flatpak installations, including secondary libraries.
- Steam AppID launching instead of starting Windows games outside Steam/Proton.
- Read-only Linux `/proc` game detection, with status refreshes.
- One-time Proton launch-option helper for mashups needing environment variables and `winhttp.dll`/`version.dll` overrides; does not overwrite Steam settings.
- `melty://` Google sign-in callback, GNOME/KDE desktop shortcut, and window/icon integration.
- Graphical runtime download and multilingual installer (English, Brazilian Portuguese, Spanish, French, German, Italian, Japanese).
- A Next.js-aware page-loading guard for the Linux Electron runtime, with bounded hydration retries and clear diagnostic messages in `~/.config/Melty/updater.log`.

## Requirements

- Linux x86_64 and a supported graphical desktop (GNOME or KDE; X11/XWayland for the installer).
- `bash`, `curl` or `wget`, `unzip`, `xdg-mime` (or `gio`), and a working network connection for the initial Electron download.
- Steam installed for Steam-based mashups. Windows games should launch through Steam + Proton.
- Electron 39.8.10 is downloaded **on first launch**, unless already installed under `~/.local/share/Melty/Programs/Electron/39.8.10/`.

## Download / installation

Download **`Melty-Linux-1.0.76-Fix9-WebGuard-Installer-x86_64.AppImage`** from the project's GitHub Releases, then:

```bash
chmod +x Melty-Linux-1.0.76-Fix9-WebGuard-Installer-x86_64.AppImage
./Melty-Linux-1.0.76-Fix9-WebGuard-Installer-x86_64.AppImage
```

Click **Install & Open**. This installs the underlying AppImage in `~/.local/share/Melty/Melty.AppImage`, sets executable permissions, adds a GNOME/KDE menu entry and icon, and registers the `melty://` protocol without root. It saves the previous application binary under `~/.local/share/Melty/Backups/`. Updates are blocked while the Melty Electron runtime is open.

For headless or scripted installs:

```bash
./Melty-Linux-1.0.76-Fix9-WebGuard-Installer-x86_64.AppImage --verify
./Melty-Linux-1.0.76-Fix9-WebGuard-Installer-x86_64.AppImage --install-only
```

Or run `Melty-Linux-1.0.76-Fix9-WebGuard-x86_64.AppImage` directly to launch/register the desktop integration.

## Build locally

Obtain the **unmodified official** `Melty-Setup-1.0.76.exe` yourself. Do not place it in the Git repository.

Build dependencies: Python 3, Node.js, C compiler, `patch`, `pkg-config`, `libarchive` development headers, X11 development headers, and `7z`, `7zz` or `7za`. On Arch-based systems the relevant packages are typically `base-devel`, `python`, `nodejs`, `p7zip`/`7zip`, `libarchive`, and `libx11`.

```bash
./tools/build.sh /path/to/Melty-Setup-1.0.76.exe ./dist
```

This builds **both** the direct-run AppImage and the GUI installer AppImage. The build is pinned to the upstream v1.0.76 main bundle's SHA-256 and fails closed if upstream changes it. Any subsequent Melty release requires reviewing and updating the patches; do not blindly reapply them to a newer executable.

Run local bridge/i18n tests with:

```bash
./tests/run.sh
```

## Paths and diagnostics

| Purpose | Default path |
| --- | --- |
| Installed AppImage | `~/.local/share/Melty/Melty.AppImage` |
| Electron runtime | `~/.local/share/Melty/Programs/Electron/39.8.10/` |
| Mod/Proton helper | `~/.local/share/Melty/Programs/Tools/` |
| Previous binaries | `~/.local/share/Melty/Backups/` |
| Desktop shortcut | `~/.local/share/applications/gg.melty.app.desktop` |
| Launcher logs | `~/.local/state/melty-linux-port/launcher.log` |
| Installer logs | `~/.local/state/melty-linux-port/installer.log` |

Diagnostic commands:

```bash
~/.local/share/Melty/Melty.AppImage --diagnostico
~/.local/share/Melty/Melty.AppImage --diagnostico-steam
~/.local/share/Melty/Melty.AppImage --diagnostico-processos
~/.local/share/Melty/Melty.AppImage --diagnostico-mods
```

To troubleshoot rendering, you can temporarily use `MELTY_DISABLE_GPU=1` when launching the direct AppImage. This option is not a fix for all GPU drivers.

## Compatibility and known limits

The desktop shell, native Steam discovery, Proton bridge, and package installation have automated or simulated tests. **Not every game mashup is verified with Melty 1.0.76**, and some Windows-specific helper executables cannot run natively on Linux. A Proton DLL hook alone does not guarantee that two different games can communicate successfully. Melty's hosted web UI and Google service remain controlled by the upstream project.

The port does **not** automatically apply upstream Windows application updates. New official releases require a reviewed Linux rebuild. Never install the official Windows executable into the Linux installation directory.

## Attribution and licensing

The Linux integration code in this repository is MIT licensed; see [`LICENSE`](LICENSE). Melty and its original application assets remain the property of their respective owners. This repository is not affiliated with or endorsed by Melty. **If you publish binaries built from the original Windows installer, ensure you have the right to redistribute the original Melty components**; the port's MIT license does not grant permission for upstream code or assets. See [`NOTICE.md`](NOTICE.md).

## Web rendering diagnostics (Fix 10)

If the web page shows **Melty could not load** even when the navigation and document checks pass, this build adds read-only diagnostics to the existing launcher log at `~/.local/state/melty-linux-port/launcher.log`. Look for `[melty-linux-webdiag]`.

These diagnostics record renderer warnings/errors (with common credentials redacted) and the presence of the site's root and `window.meltyDesktop` API. No browser cache or login data is cleared. This release **does not claim to fix upstream server-side errors**: it is intended to identify the exact failure before modifying Steam/Proton integrations.
