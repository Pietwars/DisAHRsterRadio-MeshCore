const assert=require('node:assert/strict'),fs=require('node:fs'),http=require('node:http'),path=require('node:path'),os=require('node:os'),{execFileSync}=require('node:child_process');
const {chromium}=require('playwright');
const folder=fs.mkdtempSync(path.join(os.tmpdir(),'dar-offline-'));
execFileSync('python3',['tools/build_pages.py',folder]);
const mime={'.html':'text/html','.js':'text/javascript','.json':'application/json','.webmanifest':'application/manifest+json','.gz':'application/gzip','.png':'image/png','.webp':'image/webp'};
const server=http.createServer((req,res)=>{
 const name=new URL(req.url,'http://localhost').pathname;
 if(!name.startsWith('/mesh/')){res.writeHead(404);return res.end();}
 const rel=name.slice(6)||'index.html';const file=path.join(folder,rel);
 if(!file.startsWith(folder+path.sep)){res.writeHead(403);return res.end();}
 if(!fs.existsSync(file)||!fs.statSync(file).isFile()){res.writeHead(404);return res.end();}
 res.setHeader('Content-Type',mime[path.extname(file)]||'application/octet-stream');res.end(fs.readFileSync(file));
});
(async()=>{
 await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
 const base=`http://127.0.0.1:${server.address().port}/mesh/`;
 const browser=await chromium.launch({headless:true});
 try{
  const context=await browser.newContext();const page=await context.newPage();
  await page.goto(base+'?demo=1');
  await page.waitForFunction(()=>document.querySelector('#offline-state').textContent.includes('bereit'));
  await page.waitForFunction(()=>!!navigator.serviceWorker.controller);
  const urls=await page.evaluate(async()=>{const names=await caches.keys();return (await Promise.all(names.filter(n=>n.startsWith('dar-demo-shell:')).map(async n=>(await (await caches.open(n)).keys()).map(r=>r.url)))).flat();});
  assert.equal(urls.length,14);assert.equal(urls.filter(u=>u.endsWith('.json.gz')).length,5);
  assert(urls.every(u=>!u.includes('/api/')&&!u.includes('/flasher/')));
  await context.setOffline(true);await page.reload();
  await page.waitForFunction(()=>document.querySelector('#offline-state').textContent.includes('Kein Versand'));
  const maps=await page.evaluate(async()=>{
   const names=['ahrtal','altenahr','dernau','heimersheim','sinzig'];const results=[];
   for(const name of names){const r=await fetch('map/'+name+'.json.gz');const json=await new Response(r.body.pipeThrough(new DecompressionStream('gzip'))).json();results.push(!!json);}
   return results;
  });assert(maps.every(Boolean));
  const apiFailed=await page.evaluate(async()=>{try{await fetch('/api/state');return false;}catch{return true;}});
  assert.equal(apiFailed,true);
  await context.setOffline(false);await context.close();
  console.log('PASS: Chromium offline reload and all five maps, no cached device API');
 }finally{await browser.close();server.close();fs.rmSync(folder,{recursive:true,force:true});}
})().catch(error=>{console.error(error);server.close();fs.rmSync(folder,{recursive:true,force:true});process.exitCode=1;});
