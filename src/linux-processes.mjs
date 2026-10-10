// Melty Linux: read-only process discovery for native Steam games and Proton.
// Reads only /proc; it never sends environment variables or command lines to any service.
import { readdir, readFile, readlink } from 'node:fs/promises';
import { basename } from 'node:path';

const isSteamFolder = (path) => /(?:^|\/)steamapps\/common\/[^/]+(?:\/|$)/i.test(path);
const ignored = /^(?:steam(?:webhelper|service)?|steam-runtime[^/]*|pressure-vessel[^/]*|flatpak(?:-spawn)?|bwrap|pv-bwrap|proton|umu(?:-run|-launcher)?|wine(?:64)?(?:-preloader)?|wineserver|explorer\.exe|services\.exe|rpcss\.exe|plugplay\.exe|svchost\.exe|conhost\.exe|start\.exe|cmd\.exe|rundll32\.exe|regsvr32\.exe|bash|sh|python(?:3(?:\.\d+)?)?|gamemoderun|reaper|steam-launch-wrapper|steam-runtime-supervisor|gameoverlayui|crashhandler(?:64)?|steamerrorreporter|systemd|electron)$/i;
const genericExe = /^(?:wine(?:64)?|.*\.dll|.*\.so|.*\.sh)$/i;
const numericSteam = /^(?:[1-9]\d{0,11})$/;
const nulTokens = (buffer) => buffer.toString('utf8').split('\0').filter(Boolean);
const normalized = (text) => String(text || '').replace(/\\/g, '/').replace(/^z:/i, '').replace(/\/+/, '/');
const safeRead = async (path) => readFile(path).catch(() => Buffer.alloc(0));
const safeLink = async (path) => readlink(path).catch(() => '');
const steamIdFromEnv = (tokens) => {
  for (const key of ['SteamAppId', 'SteamGameId']) {
    const value = tokens.find((entry) => entry.startsWith(`${key}=`))?.slice(key.length + 1);
    if (value && numericSteam.test(value)) return value;
  }
  return undefined;
};
function likelyGameArgument(argumentsList, comm) {
  // A Windows process launched through Wine normally keeps the original .exe in argv[0].
  // Never attribute a child game to a Proton/bash/Steam *launcher* just because
  // a .exe appears later on its command line.
  if (!argumentsList.length || (ignored.test(comm) && !/^wine(?:64)?-preloader$/i.test(comm))) return '';
  const first = normalized(argumentsList[0].replace(/^"|"$/g, ''));
  const base = basename(first);
  if (/\.exe$/i.test(base) && !ignored.test(base) && !genericExe.test(base)) return first;
  if (/\.exe$/i.test(comm) && !ignored.test(comm)) {
    const match = argumentsList.find((arg) => {
      const p = normalized(arg);
      return /\.exe$/i.test(p) && basename(p).toLowerCase() === comm.toLowerCase();
    });
    return normalized(match) || '';
  }
  return '';
}
export async function readLinuxGameProcesses(options = {}) {
  const root = options.procRoot || '/proc';
  const names = (await readdir(root).catch(() => [])).filter((name) => /^[1-9]\d*$/.test(name));
  const rows = [];
  let next = 0;
  const work = async () => {
    while (next < names.length) {
      const pid = Number(names[next++]);
      const base = `${root}/${pid}`;
      const [commBuffer, argvBuffer, exe] = await Promise.all([
        safeRead(`${base}/comm`), safeRead(`${base}/cmdline`), safeLink(`${base}/exe`)
      ]);
      const comm = commBuffer.toString('utf8').trim();
      const argv = nulTokens(argvBuffer);
      if (!comm && !argv.length) continue;
      if (ignored.test(comm) && !/^wine(?:64)?-preloader$/i.test(comm)) continue;
      const exeBase = basename(exe);
      const gameArg = likelyGameArgument(argv, comm);
      const nativePath = isSteamFolder(exe) ? exe : '';
      const windowsPath = isSteamFolder(gameArg) ? gameArg : '';
      // A game may run through Steam with an executable outside steamapps/common,
      // but its process name must be non-helper and its SteamAppId must be present.
      const suspectedGame = Boolean(gameArg || nativePath || /\.exe$/i.test(comm) ||
        (!ignored.test(exeBase) && isSteamFolder(argv.join(' '))));
      if (!suspectedGame) continue;
      const env = nulTokens(await safeRead(`${base}/environ`));
      const steamAppId = steamIdFromEnv(env);
      const cwd = await safeLink(`${base}/cwd`);
      if (!steamAppId && !windowsPath && !nativePath && !isSteamFolder(cwd)) continue;
      const candidatePath = windowsPath || nativePath ||
        (isSteamFolder(cwd) ? cwd : gameArg || exe);
      const name = (/\.exe$/i.test(comm) ? comm : gameArg ? basename(gameArg) : exeBase || comm);
      if (ignored.test(name) || (!steamAppId && !isSteamFolder(candidatePath))) continue;
      rows.push({
        ProcessId: pid,
        Name: name,
        ExecutablePath: candidatePath,
        ParentProcessId: null,
        CommandLine: argv.join(' '),
        CreationDate: null,
        MainWindowTitle: null,
        MainWindowHandle: 0,
        IsForeground: false,
        ...(steamAppId ? { SteamAppId: steamAppId } : {})
      });
    }
  };
  await Promise.all(Array.from({ length: Math.min(20, names.length) }, work));
  rows.sort((a,b) => a.ProcessId - b.ProcessId);
  return rows;
}
export function applyLinuxRunningInventory(items, rows) {
  const appIds = new Set(rows.map((row) => row.SteamAppId).filter(Boolean));
  const normalizedPaths = rows.map((row) => normalized(row.ExecutablePath).toLowerCase());
  for (const item of items) {
    const id = item.storeAppIds?.steam;
    const install = normalized(item.installDir).replace(/\/+$/, '').toLowerCase();
    item.running = Boolean((id && appIds.has(String(id))) ||
      (install && normalizedPaths.some((path) => path.startsWith(`${install}/`))));
  }
  return items;
}
if (process.argv.includes('--diagnostico-processos')) {
  const rows = await readLinuxGameProcesses();
  console.log(`Melty — Linux/Proton game processes: ${rows.length}`);
  for (const row of rows) console.log(`PID ${row.ProcessId} | Steam ${row.SteamAppId || '—'} | ${row.Name} | ${row.ExecutablePath || '?'}`);
  if (!rows.length) console.log('Nenhum processo de jogo da Steam/Proton identificado neste momento.');
}
