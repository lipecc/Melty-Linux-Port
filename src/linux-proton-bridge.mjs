/** Steam/Proton environment bridge for Melty Linux. Does not edit Steam configuration. */
import { mkdir, writeFile, readFile, chmod } from 'node:fs/promises';
import { existsSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { homedir } from 'node:os';
const data = () => process.env.XDG_DATA_HOME || join(homedir(), '.local/share');
const safeEnvName = /^[A-Z_][A-Z_0-9]{0,79}$/;
const safeSteamId = /^[1-9][0-9]{0,9}$/;
const shQuote = s => `'${String(s).replaceAll("'", "'\\''")}'`;
const toolsRoot = () => join(data(), 'Melty', 'Programs', 'Tools');
const helper = `#!/bin/sh
# Melty bridge: started by Steam as a launch-option wrapper, not instead of Steam.
app_id="\${1:-}"; shift || exit 2
case "$app_id" in ''|*[!0-9]*) echo 'Invalid Melty Steam ID' >&2; exit 2;; esac
if [ "$#" -lt 1 ]; then echo 'Steam command missing' >&2; exit 2; fi
state="\${XDG_DATA_HOME:-$HOME/.local/share}/Melty/Programs/Tools/steam-env/$app_id.env"
if [ -f "$state" ]; then
  # This file is generated locally by Melty and mode 0600, never from remote shell text.
  . "$state"
fi
exec "$@"
`;
export function convertProtonEnvValue(value) {
  if (typeof value !== 'string') return '';
  // The Wine Z: drive maps to Linux /. Return a Windows path for Win32 mods.
  if (value.startsWith('/') && existsSync(value)) return `Z:${value.replaceAll('/', '\\')}`;
  return value;
}
export async function prepareSteamProtonBridge({appId, vars={}, gameDir=''}) {
  const id=String(appId);
  if (!safeSteamId.test(id) || Number(id)>4294967295) throw new Error('Invalid Steam ID');
  const root=toolsRoot(), stateDir=join(root,'steam-env');
  await mkdir(stateDir,{recursive:true,mode:0o700});
  const binary=join(root,'melty-steam-launch');
  await writeFile(binary,helper,{mode:0o700});
  await chmod(binary,0o700);
  const env={};
  for (const [key,value] of Object.entries(vars)) {
    if (!safeEnvName.test(key) || ['PATH','LD_PRELOAD','LD_LIBRARY_PATH','HOME','SHELL','IFS'].includes(key)) continue;
    if (typeof value !== 'string' || value.length>3000 || /[\0\r\n]/.test(value)) continue;
    env[key]=convertProtonEnvValue(value);
  }
  const dlls=['winhttp','version'].filter(name=> gameDir && existsSync(join(gameDir,`${name}.dll`)));
  const overrides=(env.WINEDLLOVERRIDES||'').split(';').filter(Boolean);
  for (const dll of dlls) if (!overrides.some(item=>item.split('=')[0].toLowerCase()===dll)) overrides.push(`${dll}=n,b`);
  if (overrides.length) env.WINEDLLOVERRIDES=overrides.join(';');
  const configPath=join(stateDir, `${id}.env`);
  const content=['# Generated for a Steam game by Melty; sourced by the local shell wrapper.'];
  for (const [key,value] of Object.entries(env)) content.push(`export ${key}=${shQuote(value)}`);
  await writeFile(configPath,content.join('\n')+'\n',{mode:0o600});
  await chmod(configPath,0o600);
  const launchOptions=`${shQuote(binary)} ${id} %command%`;
  const ackFile=join(stateDir,`${id}.confirmed`);
  const confirmed=(await readFile(ackFile,'utf8').catch(()=>'' )).trim()===launchOptions;
  return {appId:id,launchOptions,confirmed,binary,configPath,keys:Object.keys(env)};
}
export async function acceptSteamProtonBridge(info) {
  const dir=dirname(info.configPath);
  await writeFile(join(dir,`${info.appId}.confirmed`),info.launchOptions+'\n',{mode:0o600});
}
