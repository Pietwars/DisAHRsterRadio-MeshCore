// Browser contract only: never opens a serial port or writes a device.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const http = require('node:http');
const path = require('node:path');
const crypto = require('node:crypto');
const {chromium} = require('playwright');
const root = path.resolve(__dirname, '..');
const cfg = JSON.parse(fs.readFileSync(path.join(root, 'docs/flasher/flasher.json')));
const image = Buffer.alloc(64); image[0] = 0xe9; image[12] = 9;
const sha = crypto.createHash('sha256').update(image).digest('hex');
const html = fs.readFileSync(path.join(root, 'docs/flasher/index.html'), 'utf8').replace(/[0-9a-f]{64}/g, sha);
const manifests = {};
for (const device of cfg.devices) for (const [mode, manifest] of Object.entries(device.modes)) {
  manifests[`/manifest-${device.id}-${mode}.json`] = manifest;
}
let fault = '', downloads = 0;
const server = http.createServer((req, res) => {
  res.setHeader('Cache-Control', 'no-store');
  if (req.url === '/') {res.setHeader('Content-Type', 'text/html'); res.end(html);}
  else if (req.url === '/integrity.mjs') {
    res.setHeader('Content-Type', 'text/javascript'); res.end(fs.readFileSync(path.join(root, 'docs/flasher/integrity.mjs')));
  } else if (manifests[req.url]) {
    const m = JSON.parse(JSON.stringify(manifests[req.url]));
    if (fault === 'manifest') m.builds[0].parts[0].offset += 1;
    res.setHeader('Content-Type', 'application/json'); res.end(JSON.stringify(m));
  } else if (req.url.endsWith('.bin')) {
    downloads++;
    if (fault === 'download') {res.writeHead(503); res.end();}
    else if (fault === 'corrupt') res.end(Buffer.from('CORRUPT'));
    else res.end(image);
  } else {res.writeHead(404); res.end();}
});
(async () => {
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  const base = `http://127.0.0.1:${server.address().port}/`;
  const browser = await chromium.launch({headless: true});
  try {
    const page = await browser.newPage({viewport: {width: 1280, height: 900}});
    // Keep this UI contract offline; real ESP Web Tools compatibility is documented
    // against its pinned source. The stub must never expose a serial/flash API.
    await page.route('https://unpkg.com/**', route => route.fulfill({contentType:'text/javascript', body:"customElements.define('esp-web-install-button', class extends HTMLElement {});"}));
    await page.goto(base);
    assert.equal(await page.locator('esp-web-install-button:visible').count(), 0);
    for (const manifest of Object.keys(manifests)) {
      const name = manifest.slice(1);
      await page.locator(`button[data-prepare="${name}"]`).click();
      const installer = page.locator(`esp-web-install-button[data-manifest="${name}"]`);
      await installer.waitFor({state: 'visible'});
      assert.equal(await installer.locator('button[slot="activate"]').isEnabled(), true);
      assert.match(await installer.getAttribute('manifest'), /^blob:/);
      // Network firmware changes after preparation cannot change the selected bytes.
      fault = 'corrupt';
      const verified = await installer.evaluate(async el => {
        const m = await (await fetch(el.getAttribute('manifest'))).json();
        const bytes = await (await fetch(m.builds[0].parts[0].path)).arrayBuffer();
        const digest = await crypto.subtle.digest('SHA-256', bytes);
        return Array.from(new Uint8Array(digest), b => b.toString(16).padStart(2,'0')).join('');
      });
      assert.equal(verified, sha); fault = '';
    }
    assert.equal(downloads, 6);
    for (const bad of ['manifest', 'download', 'corrupt']) {
      fault = bad; await page.goto(base);
      await page.locator('button[data-prepare="manifest-tdeck-update.json"]').click();
      await page.waitForFunction(() => /gesperrt|nicht|passt/.test(document.getElementById('check-tdeck-update').textContent));
      assert.equal(await page.locator('esp-web-install-button:visible').count(), 0);
      assert.equal(await page.locator('esp-web-install-button button:not([disabled])').count(), 0);
      // Retry after correcting the failure must work.
      fault = ''; await page.locator('button[data-prepare="manifest-tdeck-update.json"]').click();
      await page.locator('esp-web-install-button[data-manifest="manifest-tdeck-update.json"]').waitFor({state: 'visible'});
    }
    await page.close();
    console.log('PASS: browser blocks installation until verification, all six modes, immutable snapshots and failed-download retry');
  } finally {await browser.close(); server.close();}
})().catch(error => {console.error(error); server.close(); process.exitCode = 1;});
