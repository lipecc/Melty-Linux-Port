# Melty Linux Port 1.0.72 — multilingual installer

Suggested GitHub release tag: `v1.0.72-linux.1`

## What's new

- Graphical first-run installer with Install & Open / Run Only / Cancel.
- Automatic GNOME and KDE application-menu entry, icon, executable permissions, and `melty://` authentication callback registration, without root privileges.
- Installer and Electron download progress window localized into English, Brazilian Portuguese, Spanish, French, German, Italian, and Japanese; remembers language selection.
- Preserves the working Steam/Proton/Dead Ascent integration from Fix 7.
- Reuses existing Electron runtime in `~/.local/share/Melty/Programs/Electron/39.8.10/` when installed.
- Includes `--install`, `--uninstall`, `--set-language` and diagnostics.

## Installation

Download `Melty-Linux-1.0.72-Installer-Multilang-x86_64.AppImage`, allow execution, open it and choose **Install & Open**. The installer operates per-user, without `sudo`.

```bash
chmod +x Melty-Linux-1.0.72-Installer-Multilang-x86_64.AppImage
./Melty-Linux-1.0.72-Installer-Multilang-x86_64.AppImage
```

**Compatibility:** x86_64 Linux with GNOME/KDE or an X11/XWayland desktop. Electron 39.8.10 is downloaded on first launch. This is an unofficial community port. Its AppImage contains the upstream proprietary Melty application; check redistribution permission before publishing this binary to GitHub Releases.
