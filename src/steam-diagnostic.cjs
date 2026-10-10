#!/usr/bin/env node
/* Read-only Linux Steam diagnostic. It never modifies game/Steam files. */
const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');
const home = os.homedir();
const dataHome = process.env.XDG_DATA_HOME || path.join(home,'.local','share');
const envRoots = (process.env.MELTY_STEAM_LIBRARY_ROOTS || '').split(':').filter(Boolean);
const roots = [...new Set([...envRoots,
 path.join(dataHome,'Steam'),path.join(home,'.steam','steam'),path.join(home,'.steam','root'),
 path.join(home,'.steam','debian-installation'),path.join(home,'snap','steam','common','.local','share','Steam'),
 path.join(home,'.var','app','com.valvesoftware.Steam','data','Steam'),
 path.join(home,'.var','app','com.valvesoftware.Steam','.local','share','Steam')])];
const field=(str,key)=>[...str.matchAll(new RegExp('"'+key+'"\\s+"([^"\\r\\n]*)"','ig'))].map(x=>x[1]);
const existsDirectory=p=>{try{return fs.statSync(p).isDirectory()}catch{return false}};
const reported = new Set();
let found=0;
console.log('Melty Linux: Steam/Proton diagnostic (read-only)');
console.log('Steam executable: ', ['/usr/bin/steam','/usr/games/steam','/usr/local/bin/steam', '/var/lib/flatpak/exports/bin/com.valvesoftware.Steam', path.join(dataHome,'flatpak','exports','bin','com.valvesoftware.Steam')].filter(p=>fs.existsSync(p)).join(' | ') || 'not found in standard paths');
for(const root of roots) {
 if (!existsDirectory(root)) continue;
 console.log('\nSteam root:',root);
 const index=path.join(root,'steamapps','libraryfolders.vdf');
 const indexed=fs.existsSync(index) ? field(fs.readFileSync(index,'utf8'),'path').map(p=>p.replace(/\\\\/g,'\\')) : [];
 if (indexed.length) console.log('Bibliotecas configuradas:', indexed.join(' | '));
 for(const library of [root,...indexed]) {
  if(reported.has(library))continue;reported.add(library);
  const apps=path.join(library,'steamapps');
  if(!existsDirectory(apps)){console.log('No steamapps directory:',apps);continue;}
  const names=fs.readdirSync(apps).filter(n=>/^appmanifest_\d+\.acf$/i.test(n));
  console.log('Library:',library,'— manifests:',names.length);
  for(const name of names) {
   try {
    const contents=fs.readFileSync(path.join(apps,name),'utf8');
    const title=field(contents,'name')[0] || name;
    const dir=field(contents,'installdir')[0];
    const installed=dir&&existsDirectory(path.join(apps,'common',dir));
    console.log(`  ${installed?'[INSTALADO]':'[SEM PASTA]'} ${title}`);
    if(installed) {
      found++;
      if (process.argv.includes('--mods')) {
        const gameDir=path.join(apps,'common',dir);
        const dll=['winhttp.dll','version.dll','doorstop_config.ini','BepInEx','MelonLoader','dxgi.dll'].filter(x=>fs.existsSync(path.join(gameDir,x)));
        if(dll.length) {
          console.log('    Mod/loader na pasta:',dll.join(', '));
          if(dll.includes('winhttp.dll')) console.log('    Proton: if the mod fails to load, set Steam launch options to: WINEDLLOVERRIDES="winhttp=n,b" %command%');
          if(dll.includes('version.dll')) console.log('    Proton: if the mod fails to load, try: WINEDLLOVERRIDES="version=n,b" %command%');
          if(dll.includes('dxgi.dll')) console.log('    WARNING: dxgi.dll can conflict with DXVK. Check before setting overrides.');
        }
        const compat=path.join(apps,'compatdata',String(field(contents,'appid')[0]||name.match(/\d+/)?.[0]),'pfx');
        console.log('    Proton prefix:',fs.existsSync(compat)?compat:'not found in this library');
      }
    }
   } catch(e) {console.log('  [ERRO DE LEITURA]',name,e.code||e.message)}
  }
 }
}
console.log('\nInstalled games with existing directories:',found);
if(found===0) console.log('Tip: Steam > Settings > Storage. Check that the game drive is mounted and .acf files are readable. You may also set MELTY_STEAM_LIBRARY_ROOTS.');
