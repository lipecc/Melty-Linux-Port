# Melty Linux 1.0.76 Fix 10 — Web Diagnostics

- Added read-only Electron renderer console diagnostics and bridge-health snapshots.
- Preserves Fix 9 Linux web-guard, Steam/Proton integration, OAuth, GNOME launcher and installer.
- No automatic cache deletion, changes to game files or bypass of server-side errors.
- To report an error: attach `~/.local/state/melty-linux-port/launcher.log` (filter for `melty-linux-webdiag`).
- Not verified end-to-end against the authenticated Melty website.
