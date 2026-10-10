# Melty Linux Port

An **unofficial, community-built Linux compatibility port** of the [Melty](https://melty.gg/) desktop application. It integrates the original desktop experience with native Linux Steam, Proton, and GNOME/KDE desktops.

> **Status:** Linux port of upstream Melty **1.0.72**. Tested in a controlled Linux environment, with PEAK/Dead Ascent reported working on a real Linux computer. Other mashups may require additional Wine/Proton work. This project is not affiliated with the Melty developers, Steam, or Valve.

## Features

- Graphical, **no-root AppImage installer** with **Install & Open**, **Run Only**, and **Cancel** actions.
- Creates an application-menu shortcut, installs the Melty icon, and registers `melty://` deep links for external Google sign-in.
- Uses a stable per-user AppImage path, so future updates do not break authentication callbacks.
- Detects games in native Steam, Flatpak Steam, and additional registered Steam libraries.
- Tracks Steam/Proton game processes, including games launched through Proton.
- Starts supported Steam games **through Steam**, preserving Steam launch options and Proton selection.
- Provides a **Steam/Proton environment bridge** for mashups needing variables such as `DEADASCENT_L4D2_DIR` and DLL overrides such as `winhttp=n,b`.
- Downloads the matching Electron Linux runtime on first use, with a native progress window, then reuses the installation.
- Diagnostic commands for Steam libraries, running game processes, and mod loaders.
- Localized Linux installer/downloader in **English, Brazilian Portuguese, Spanish, French, German, Italian, and Japanese**.

The web interface loaded from melty.gg is **not bundled or translated by this project**. The port forwards its language preference to Electron, but available in-app translations depend on the Melty website itself.

## Download and install

1. Download `Melty-Linux-1.0.72-x86_64.AppImage` from this repository's **Releases** page.
2. Allow the downloaded file to execute (through your file manager, or using `chmod +x`).
3. Open it. The graphical installer will appear. Pick your language and **Install & Open**.
4. The installer copies itself to `~/.local/share/Melty/Melty.AppImage`, creates an app-menu entry, installs the icon, and registers `melty://` automatically. **No `sudo` required.**
5. At first launch, let the app download the official Electron 39.8.10 Linux runtime (~110 MB). Later launches reuse this runtime.

Terminal example:

```bash
chmod +x ./Melty-Linux-1.0.72-x86_64.AppImage
./Melty-Linux-1.0.72-x86_64.AppImage
```

The GUI uses X11/XWayland. On a Wayland-only or headless session without XWayland, the launcher falls back to per-user installation in the terminal.

### Language selection

The installer initially uses your Linux system language (English fallback). Use the arrows at the top of the installer to switch languages. It remembers your choice in `~/.config/Melty/language` (or `$XDG_CONFIG_HOME/Melty/language`).

You may also select the language manually:

```bash
./Melty-Linux-1.0.72-x86_64.AppImage --set-language es
```

Accepted codes: `en`, `pt-BR`, `es`, `fr`, `de`, `it`, `ja`. An explicit `MELTY_LANG` environment variable overrides the saved language. Text rendering for Japanese requires a suitable font installed on the desktop.

### Managing the installation

```bash
# Install the desktop integration without opening the UI
./Melty-Linux-1.0.72-x86_64.AppImage --install

# Inspect registered paths and runtime state
./Melty-Linux-1.0.72-x86_64.AppImage --diagnostic

# Read-only Steam/game diagnostics
./Melty-Linux-1.0.72-x86_64.AppImage --steam-diagnostic
./Melty-Linux-1.0.72-x86_64.AppImage --process-diagnostic
./Melty-Linux-1.0.72-x86_64.AppImage --mod-diagnostic

# Remove the shortcut, installed AppImage, and icon; retain game/user data
./Melty-Linux-1.0.72-x86_64.AppImage --uninstall
```

Installed files are located in:

```text
~/.local/share/Melty/
├── Melty.AppImage
├── bin/melty-linux
├── Downloads/
└── Programs/
    ├── Electron/39.8.10/
    └── Tools/
~/.local/share/applications/gg.melty.app.desktop
~/.local/share/icons/hicolor/512x512/apps/gg.melty.app.png
~/.config/Melty/language
~/.local/state/melty-linux-port/launcher.log
```

## Steam/Proton mashups

Most ordinary Steam games should be launched using Steam, **not by directly executing their Windows `.exe`**. Games that need mod-specific environment variables are handled through a launch-option wrapper generated under `~/.local/share/Melty/Programs/Tools/`.

For example, a PEAK / Dead Ascent mashup may require a `DEADASCENT_L4D2_DIR` path and a Wine override for `winhttp.dll`. When required, Melty prompts you to add an option to the game's **Steam > Properties > General > Launch Options**. Review and preserve any options you already use; the launcher does **not** silently edit Steam's configuration. Only files belonging to the mod and the explicitly configured games should be modified by the normal Melty flow.

**Security:** Mods, third-party game loaders, and injected DLLs are executable code. Install mods only from developers and sources you trust. This project does not bypass DRM, Steam authentication, or anticheat systems.

## Building from source

The repository contains the **original Linux compatibility code**, small upstream patch files, a native AppImage bootstrap, and build/test scripts. It intentionally does **not** include proprietary upstream Melty code, game files, or an Electron binary.

You must provide your own legitimate copy of the official **Melty 1.0.72 Windows installer** (`Melty-Setup-1.0.72.exe`). The build requires Linux x86_64, Python 3, a C compiler, `patch`, `pkg-config`, `libarchive` development files, X11 development files, and `7z`/`7za`. For example on Arch Linux / EndeavourOS:

```bash
sudo pacman -S --needed base-devel python p7zip libarchive libx11 pkgconf patch
```

Clone this repository and run:

```bash
./tools/build.sh ~/Downloads/Melty-Setup-1.0.72.exe
# Generated artifact: dist/Melty-Linux-1.0.72-x86_64.AppImage
```

The build verifies the SHA-256 of the exact upstream Electron bundle. If upstream changes the installer, the build stops rather than silently applying a patch to a different version.

Optional tests (Node.js 20+):

```bash
./tests/run.sh
```

### Repository layout

| Path | Purpose |
| --- | --- |
| `src/` | Native installer/downloader, process and Steam/Proton integration, and runtime |
| `src/i18n.h` | Installer and downloader translations |
| `linux/AppRun` | AppImage entry point, installation, Electron bootstrap, diagnostics |
| `patches/` | Minimal patches applied to the official upstream Electron archive |
| `tools/` | Extraction, patching, repacking, and AppImage build utilities |
| `tests/` | Linux Steam/Proton bridge tests |

## Releasing on GitHub

Source code belongs in the main repository; prebuilt AppImages belong under **GitHub Releases**. Suggested tag: `v1.0.72-linux.1`. Add the generated `.AppImage` as a release asset, not a committed binary. Do **not** add the original Windows installer or unmodified upstream ASAR files to git.

Before distributing a binary containing Melty's original proprietary components, verify that upstream's license and/or their express permission allows redistribution. The MIT license in this repository covers **only the independently written Linux port code**, not Melty, Valve/Steam, third-party games, or their assets.

## Troubleshooting

If the window fails to load, consult `~/.local/state/melty-linux-port/launcher.log`. If game detection fails, use `--steam-diagnostic` and `--process-diagnostic`. If a mashup fails, inspect `--mod-diagnostic` and the relevant game's Proton log. Include minimal, redacted output in an issue; **never post authentication cookies, OAuth callback tokens, or personal paths you wish to keep private**.

## Contributions

Pull requests for translations, new Steam library layouts, distro compatibility, and Proton-specific bug fixes are welcome. Please test changes without overwriting user Steam launch options or game files and ensure regression coverage for the Steam/Proton bridge.

## License

New code for the Linux port is licensed under MIT; see [`LICENSE`](LICENSE) and [`NOTICE.md`](NOTICE.md) for scope and third-party licensing notes.
