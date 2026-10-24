// Render the current simulated portal, never a live device or real contacts.
const { chromium } = require('playwright');
const path = require('path');
const fs = require('fs');
(async () => {
  const browser = await chromium.launch({ headless: true });
  try {
    const page = await browser.newPage({ viewport: { width: 390, height: 844 }, deviceScaleFactor: 1 });
    await page.goto('http://127.0.0.1:8765/?demo=1');
    await page.waitForSelector('#qg button', { state: 'attached' });
    fs.mkdirSync('web-previews', { recursive: true });
    for (const [tab, name] of [['chat', 'chat'], ['quick', 'meldung'], ['map', 'karte'], ['board', 'board'], ['loc', 'loc'], ['info', 'info']]) {
      await page.locator(`nav button[data-tab="${tab}"]`).click();
      await page.waitForTimeout(500);
      await page.screenshot({ path: path.join('web-previews', `vorschau-${name}.jpg`), type: 'jpeg', quality: 90 });
      fs.copyFileSync(path.join('web-previews', `vorschau-${name}.jpg`), path.join('web-previews', `funktion-${tab}.jpg`));
    }
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exit(1); });
