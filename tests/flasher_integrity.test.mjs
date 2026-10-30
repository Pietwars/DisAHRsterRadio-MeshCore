import assert from 'node:assert/strict';
import {webcrypto} from 'node:crypto';
import {readFileSync} from 'node:fs';
import {prepareFirmware} from '../docs/flasher/integrity.mjs';
const clone = value => JSON.parse(JSON.stringify(value));
const image = new Uint8Array(64); image[0] = 0xe9; image[12] = 9;
const sha = async bytes => Buffer.from(await webcrypto.subtle.digest('SHA-256', bytes)).toString('hex');
const config = JSON.parse(readFileSync(new URL('../docs/flasher/flasher.json', import.meta.url)));
assert.deepEqual(config.devices.map(d => d.id).sort(), ['tdeck', 'twatch', 'tdeck-standalone', 't3s3-sx1262'].sort());
const base = 'https://example.test/flasher/';
let count = 0;
function harness(spec, {mutateManifest, bytes = image, failStatus, failDigest, failURL} = {}) {
  const blobs = new Map(), calls = [], revoked = [];
  return {blobs, calls, revoked, api: {
    fetch: async (url, options) => {
      calls.push({url, options});
      if (failStatus) return {ok: false};
      if (url.endsWith('.json')) {
        const manifest = clone(spec.manifest); mutateManifest?.(manifest);
        return {ok: true, json: async () => manifest};
      }
      return {ok: true, arrayBuffer: async () => bytes.slice().buffer};
    },
    digest: async bytes => {if (failDigest) throw Error('digest failed'); return webcrypto.subtle.digest('SHA-256', bytes);},
    createURL: blob => {if (failURL && blobs.size) throw Error('url failed'); const url = `blob:https://example.test/${++count}`; blobs.set(url, blob); return url;},
    revokeURL: url => {revoked.push(url); blobs.delete(url);}
  }};
}
for (const device of config.devices) for (const [mode, manifest] of Object.entries(device.modes)) {
  const spec = {file: `manifest-${device.id}-${mode}.json`, manifest, sha256: await sha(image)};
  const h = harness(spec);
  const result = await prepareFirmware(spec, base, h.api);
  const verified = JSON.parse(await h.blobs.get(result.manifestURL).text());
  const firmwareURL = verified.builds[0].parts[0].path;
  assert(firmwareURL.startsWith('blob:'));
  assert.deepEqual(new Uint8Array(await h.blobs.get(firmwareURL).arrayBuffer()), image);
  assert.equal(verified.builds[0].parts[0].offset, manifest.builds[0].parts[0].offset);
  assert.equal(verified.new_install_prompt_erase, mode === 'update');
  assert.equal(h.calls.length, 2); // no second network fetch of firmware during installation
  assert(h.calls.every(call => call.options.cache === 'no-store' && call.options.redirect === 'error'));
  result.dispose(); result.dispose(); assert.equal(h.blobs.size, 0);
  for (const mutateManifest of [
    m => m.builds[0].chipFamily = 'ESP32',
    m => m.builds[0].parts[0].path = 'https://evil.test/firmware.bin',
    m => m.builds[0].parts[0].offset += 1,
    m => m.builds[0].parts.push({path:'extra.bin',offset:0}),
    m => m.builds.push(clone(m.builds[0])),
    m => m.version = '0.0.0',
    m => m.new_install_prompt_erase = !m.new_install_prompt_erase,
  ]) {
    const bad = harness(spec, {mutateManifest});
    await assert.rejects(prepareFirmware(spec, base, bad.api), /Manifest passt nicht/);
    assert.equal(bad.blobs.size, 0); assert.equal(bad.calls.length, 1);
  }
  for (const options of [{bytes: new Uint8Array([1,2,3])}, {failStatus: true}, {failDigest: true}, {failURL: true}]) {
    const bad = harness(spec, options);
    await assert.rejects(prepareFirmware(spec, base, bad.api));
    assert.equal(bad.blobs.size, 0);
  }
  for (const bytes of [new Uint8Array(0), image.slice(0, 20), (() => {const b = image.slice(); b[12] = 0; return b;})()]) {
    const badSpec = {...spec, sha256: await sha(bytes)};
    const bad = harness(badSpec, {bytes});
    await assert.rejects(prepareFirmware(badSpec, base, bad.api), /ESP32-S3/);
    assert.equal(bad.blobs.size, 0);
  }
}
// The prepared and live flasher must enforce the same checks and source generation.
assert.equal(readFileSync(new URL('../docs/flasher/integrity.mjs', import.meta.url), 'utf8'),
             readFileSync(new URL('../tools/flasher-next/integrity.mjs', import.meta.url), 'utf8'));
for (const folder of ['docs/flasher', 'tools/flasher-next']) {
  const html = readFileSync(new URL(`../${folder}/index.html`, import.meta.url), 'utf8');
  const specs = JSON.parse(html.match(/<script id="flasher-integrity" type="application\/json">(.*?)<\/script>/s)[1]);
  const modeCount = config.devices.reduce((n, d) => n + Object.keys(d.modes).length, 0);
  assert.equal(Object.keys(specs).length, modeCount);
  assert.equal((html.match(/<esp-web-install-button hidden data-manifest=/g) || []).length, modeCount);
  assert.equal((html.match(/<button disabled slot="activate"/g) || []).length, modeCount);
  assert.equal((html.match(/data-prepare=/g) || []).length, modeCount);
  for (const device of config.devices) for (const [mode, manifest] of Object.entries(device.modes)) {
    const file = `manifest-${device.id}-${mode}.json`;
    assert.deepEqual(specs[file], {file, manifest, sha256: device.files[mode].sha256});
  }
}
console.log('PASS: all configured flasher modes; SHA mismatch, wrong target/offset/erase, failed downloads and immutable verified bytes');
