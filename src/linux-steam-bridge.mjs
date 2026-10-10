/** Melty Linux Steam/Proton launcher: no direct Windows executable launches. */
import { spawn } from 'node:child_process';
import { accessSync, constants } from 'node:fs';
import { basename, relative, isAbsolute, resolve } from 'node:path';

export function steamIdFromLink(link) {
  const match = /^steam:\/\/(?:rungameid|run)\/([1-9]\d{0,9})(?:[/?#]|$)/i.exec(String(link || ''));
  return match && Number(match[1]) <= 4294967295 ? match[1] : null;
}
const normalName = v => String(v || '').toLowerCase().replace(/\.exe$/,'').replace(/[^a-z0-9]/g,'');
const isInside = (root, child) => {
  const rel = relative(resolve(root),resolve(child));
  return rel !== '' && rel !== '..' && !rel.startsWith('../') && !isAbsolute(rel);
};
/** Require exact main binary evidence: a random third-party mod launcher must NOT be bypassed. */
export function matchSteamMainExecutable(file, evidence, catalogAliases = []) {
  if (typeof file !== 'string' || !/\.exe$/i.test(file)) return null;
  const exeName = basename(file).toLowerCase();
  for (const game of evidence || []) {
    if (game.launcher !== 'Steam' || !/^[1-9]\d{0,9}$/.test(String(game.storeAppId || ''))) continue;
    if (!game.installPath || !isInside(game.installPath, file)) continue;
    const matchingAlias = catalogAliases.find(x => Array.isArray(x.steamAppIds) && x.steamAppIds.some(id => String(id) === String(game.storeAppId)));
    const listed = matchingAlias?.processNames ?? [];
    const installName = basename(game.installPath);
    const safeName = new Set([normalName(game.displayName), normalName(installName)]);
    const namedMain = safeName.has(normalName(exeName));
    const exactKnownMain = listed.some(name => String(name).toLowerCase() === exeName);
    if (!namedMain && !exactKnownMain) continue;
    return { appId: String(game.storeAppId), game: game.displayName, exeName };
  }
  return null;
}

function isExec(file) { try { accessSync(file, constants.X_OK); return true; } catch { return false; } }
/** Prefer the native Steam CLI, then Flatpak launcher, then let Electron open steam://. */
export function steamCommand(candidates = null) {
  const home = process.env.HOME || '';
  const cmds = candidates ?? [
    [process.env.MELTY_STEAM_EXECUTABLE || '', []],
    ['/usr/bin/steam', []], ['/usr/games/steam', []], ['/usr/local/bin/steam', []],
    ['/var/lib/flatpak/exports/bin/com.valvesoftware.Steam', []],
    [`${home}/.local/share/flatpak/exports/bin/com.valvesoftware.Steam`, []],
    ['/usr/bin/flatpak', ['run','com.valvesoftware.Steam']]
  ];
  for (const [command, prefix] of cmds) if (command && isExec(command)) return { command, prefix };
  return null;
}
export async function launchSteamApp(appId, args = [], options = {}) {
  const id = String(appId);
  if (!/^[1-9]\d{0,9}$/.test(id) || Number(id) > 4294967295) throw new Error('Invalid Steam AppID');
  const command = options.command === undefined ? steamCommand() : options.command;
  if (command) {
    const argv = [...command.prefix, '-applaunch', id, ...args];
    const childSpawn = options.spawnImpl ?? spawn;
    await new Promise((ok, fail) => {
      const child = childSpawn(command.command, argv, { detached: true, stdio: 'ignore' });
      child.once('spawn', () => { child.unref?.(); ok(); });
      child.once('error', fail);
    });
    return { method: 'steam-cli', command: command.command, argv };
  }
  if (args.length) throw new Error('Steam command not installed; cannot safely forward extra game arguments via steam://');
  if (!options.openExternal) throw new Error('Steam command not installed');
  await options.openExternal(`steam://rungameid/${id}`);
  return { method: 'steam-uri', appId: id };
}
