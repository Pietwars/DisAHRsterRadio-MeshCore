// Browser UI with a simulated device API; never transmits radio messages.
const assert=require('node:assert/strict'),fs=require('node:fs'),http=require('node:http');
const {chromium}=require('playwright');
const html=fs.readFileSync('web/index.html');
const key='10'.repeat(32),target='d'+key.slice(0,12);
const contact=Buffer.alloc(148);contact[0]=3;Buffer.from(key,'hex').copy(contact,1);contact[33]=1;contact.write('Kontakt',100);
const state={ready:true,seq:0,channels:[],contacts:[contact.toString('hex')],diagnostics:{},room:{},gps:{enabled:false}};
let sos={storage:true,id:0,state:0,attempts:0},posts=[];
const server=http.createServer((req,res)=>{
 const name=new URL(req.url,'http://localhost').pathname;
 if(name==='/'){res.setHeader('Content-Type','text/html');return res.end(html);}
 if(name==='/api/state'){res.setHeader('Content-Type','application/json');return res.end(JSON.stringify(state));}
 if(name==='/api/history')return res.end('');
 if(name==='/api/sos'){
  if(req.method==='GET'){res.setHeader('Content-Type','application/json');return res.end(JSON.stringify(sos));}
  let body='';req.on('data',part=>body+=part);req.on('end',()=>{
   const args=Object.fromEntries(new URLSearchParams(body));posts.push(args);
   if(args.action==='start')sos={storage:true,id:1,state:6,attempts:0,token:'1234567812345678',label:'SOS: Annahme offen'};
   else sos={...sos,state:12,label:'Abgebrochen'};
   res.end('Gespeichert');
  });return;
 }
 res.writeHead(404);res.end();
});
(async()=>{
 await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
 const browser=await chromium.launch({headless:true,...(process.env.DAR_BROWSER_CHANNEL?{channel:process.env.DAR_BROWSER_CHANNEL}:{})});
 try{
  const page=await browser.newPage();await page.goto(`http://127.0.0.1:${server.address().port}/`);
  await page.locator('nav button[data-tab=private]').click();await page.waitForFunction(()=>document.querySelector('#private-target').options.length===1&&document.querySelector('#private-target').value.startsWith('d'));
  assert.equal(await page.locator('#sos-start').isDisabled(),true);assert.equal(posts.length,0);
  await page.locator('#sos-prepare').click();assert.equal(posts.length,0);
  // Separate controls plus minimum confirmation age: an immediate second tap cannot send.
  await page.locator('#sos-start').click();assert.equal(posts.length,0);
  await page.locator('#sos-prepare').click();await page.waitForTimeout(800);
  await page.locator('#sos-start').click();await page.waitForFunction(()=>document.querySelector('#sos-status').textContent.includes('Annahme offen'));
  assert.equal(posts.length,1);assert.equal(posts[0].target,target);assert.equal(posts[0].key,key);assert.equal(posts[0].share,'0');
  await page.locator('#sos-cancel').click();await page.waitForFunction(()=>document.querySelector('#sos-status').textContent.includes('Abgebrochen'));
  assert.equal(posts[1].action,'cancel');assert.equal(posts[1].id,'1');
  await page.locator('#sos-prepare').click();await page.locator('#sos-share').check();await page.waitForTimeout(800);
  await page.locator('#sos-start').click();assert.equal(posts.length,2); // changed consent requires new confirmation
  await page.goto(`http://127.0.0.1:${server.address().port}/?demo=1`);await page.locator('nav button[data-tab=private]').click();
  await page.locator('#sos-prepare').click();assert.equal(posts.length,2);assert.equal(await page.locator('#sos-start').isDisabled(),true);
  console.log('PASS: SOS browser confirmation, pinned recipient key, explicit location consent, cancel and no demo transmission');
 }finally{await browser.close();server.close();}
})().catch(error=>{console.error(error);server.close();process.exitCode=1;});
