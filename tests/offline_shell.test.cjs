const assert=require('node:assert/strict'),fs=require('node:fs'),vm=require('node:vm');
const source=fs.readFileSync('web/sw.js','utf8');
const handlers={},stores=new Map(),deleted=[],network=[];
const scope='https://example.test/mesh/';
let fail=false,claimed=false;
const key=value=>typeof value==='string'?value:value.url;
const caches={
 async open(name){if(!stores.has(name)){const data=new Map();stores.set(name,{
  async addAll(urls){if(fail)throw Error('missing asset');for(const url of urls)data.set(key(url),'cached:'+key(url));},
  async match(request){return data.get(key(request));},data
 });}return stores.get(name);},
 async keys(){return [...stores.keys()];},async delete(name){deleted.push(name);return stores.delete(name);}
};
const context=vm.createContext({URL,Request,caches,fetch:async request=>{network.push(key(request));return 'network';},
 self:{registration:{scope},clients:{claim:async()=>{claimed=true;}},addEventListener:(name,handler)=>handlers[name]=handler}});
vm.runInContext(source,context);
function lifecycle(name){let pending;handlers[name]({waitUntil:promise=>pending=promise});return pending;}
function request(url,method='GET',mode='cors'){let result;handlers.fetch({request:{url,method,mode},respondWith:p=>result=p});return result;}
(async()=>{
 fail=true;await assert.rejects(lifecycle('install'));assert.equal(claimed,false);
 fail=false;await lifecycle('install');
 const current=[...stores.keys()][0];assert(current.startsWith('dar-demo-shell:'+scope+':'));
 assert.equal(stores.get(current).data.size,14);
 for(const url of stores.get(current).data.keys())assert(!/api|flasher|firmware|\.bin/.test(url));
 stores.set('dar-demo-shell:'+scope+':old',{});
 stores.set('dar-demo-shell:https://example.test/other/:old',{});stores.set('unrelated-app',{});
 await lifecycle('activate');assert.equal(claimed,true);
 assert.deepEqual(deleted,['dar-demo-shell:'+scope+':old']);
 for(const url of [scope+'api/state','https://example.test/api/state',scope+'flasher/firmware.bin',scope+'map/unknown.json.gz','https://other.test/index.html']){
  assert.equal(request(url),undefined);assert.equal(request(url,'POST'),undefined);
 }
 assert.equal(await request(scope+'?demo=1','GET','navigate'),'cached:'+scope+'index.html');
 assert.equal(await request(scope+'map/heimersheim.json.gz'),'cached:'+scope+'map/heimersheim.json.gz');
 assert.equal(network.length,0);
 assert.equal(request(scope+'flasher/','GET','navigate'),undefined);
 const html=fs.readFileSync('web/index.html','utf8');
 assert.match(html,/if\(!DEMO\|\|!isSecureContext\|\|!\('serviceWorker' in navigator\)\)return/);
 assert.match(html,/Nur simulierte Daten/);assert.match(html,/Kein Versand an ein Funkgerät/);
 console.log('PASS: atomic offline shell, five maps, scope isolation and no API/firmware caching');
})().catch(error=>{console.error(error);process.exitCode=1;});
