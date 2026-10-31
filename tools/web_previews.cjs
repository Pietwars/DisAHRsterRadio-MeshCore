// Render the current simulated portal, never a live device or real contacts.
const { chromium } = require('playwright');
const path = require('path');
const fs = require('fs');
const assert = require('node:assert/strict');
(async () => {
  const browser = await chromium.launch({ headless: true, ...(process.env.DAR_BROWSER_CHANNEL ? {channel: process.env.DAR_BROWSER_CHANNEL} : {}) });
  try {
    const page = await browser.newPage({ viewport: { width: 390, height: 844 }, deviceScaleFactor: 1 });
    await page.goto('http://127.0.0.1:8765/?demo=1');
    await page.waitForSelector('#qg button', { state: 'attached' });
    await page.waitForFunction(() => document.querySelector('#private-target').options.length === 2);
    await page.locator('nav button[data-tab="info"]').click();
    await page.waitForFunction(()=>document.getElementById('device-name').textContent.includes('Heimersheim'));
    assert.equal(await page.locator('#device-wifi').innerText(),'DisAHRsterRadio-246F28123456');
    assert.equal(await page.locator('#device-profile').innerText(),'EU/UK Narrow');
    await page.evaluate(()=>{state.diagnostics.nodeNameHex=hx(enc.encode('<img src=x onerror=alert(1)>'));state.diagnostics.bleConnected=true;state.diagnostics.radio={frequencyMHz:869.618,bandwidthKhz:62.5,sf:9,cr:8,powerDbm:17};renderInfo()});
    assert.equal(await page.locator('#device-name img').count(),0);
    assert.match(await page.locator('#device-name').innerText(),/<img/);
    assert.equal(await page.locator('#device-profile').innerText(),'Eigenes Profil');
    assert.equal(await page.locator('#device-bluetooth').innerText(),'App verbunden');
    await page.setViewportSize({width:320,height:740});
    assert.equal(await page.locator('.device-overview').evaluate(el=>el.scrollWidth<=el.clientWidth),true);
    await page.setViewportSize({width:390,height:844});
    await page.evaluate(async()=>{lastSeq=-1;await poll()});
    await page.locator('nav button[data-tab="private"]').click();
    await page.evaluate(() => {
      state.contacts=Array.from({length:350},(_,i)=>demoContact(i.toString(16).padStart(12,'0'),1,'Kontakt '+i,0,0));
      state.contacts.push(demoContact('abcdefabcdef',2,'Repeater',0,0),demoContact('fedcbafedcba',3,'Room',0,0));
      state.privateHistoryFloor=10;targets();privateTarget='d00000000015d';document.querySelector('#private-target').value=privateTarget;
      msgs=[{t:privateTarget,seq:9,body:'Alter Verlauf',stamp:now()},{t:privateTarget,seq:11,body:'Neu angekommen',stamp:now()},{t:privateTarget,seq:12,room:true,body:'Roombeitrag',stamp:now()},{t:'c0',seq:13,body:'Kanalnachricht',stamp:now()}];renderPrivate();
      window.savedSend=send;send=async(t,text)=>{window.privateSent={t,text};return 'Test angenommen'};
    });
    assert.equal(await page.locator('#private-target option').count(),350);
    assert.equal(await page.locator('#target option[value^="d"]').count(),0);
    assert.equal(await page.locator('#private-log .m').count(),1);
    assert.match(await page.locator('#private-log').innerText(),/Neu angekommen/);
    await page.locator('#private-msg').fill('Privater Test');await page.locator('#private-send').click();
    await page.waitForFunction(()=>window.privateSent?.text==='Privater Test');
    assert.equal(await page.evaluate(()=>window.privateSent.t),'d00000000015d');
    await page.evaluate(()=>{state.contacts[349]=demoContact('00000000015d',1,'Umbenannt',0,0);targets()});
    assert.equal(await page.locator('#private-target option:checked').innerText(),'✉ Umbenannt');
    await page.evaluate(()=>{state.contacts.splice(349,1);targets()});
    assert.equal(await page.locator('#private-target option').count(),349);
    await page.evaluate(async()=>{send=window.savedSend;lastSeq=-1;await poll()});
    console.log('PASS: private pane, 350 contacts, rename/delete, channels separate, fresh history and private send target');
    fs.mkdirSync('web-previews', { recursive: true });
    for (const [tab, name] of [['chat', 'chat'], ['private', 'privat'], ['quick', 'meldung'], ['map', 'karte'], ['board', 'board'], ['loc', 'loc'], ['info', 'info']]) {
      await page.locator(`nav button[data-tab="${tab}"]`).click();
      await page.waitForTimeout(500);
      await page.screenshot({ path: path.join('web-previews', `vorschau-${name}.jpg`), type: 'jpeg', quality: 90 });
      fs.copyFileSync(path.join('web-previews', `vorschau-${name}.jpg`), path.join('web-previews', `funktion-${tab}.jpg`));
    }
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exit(1); });
