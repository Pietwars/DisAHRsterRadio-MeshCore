// SHA-256 verifies downloaded bytes, not publisher identity or hardware acceptance.
// ESP Web Tools 10.4.0 accepts blob manifests with absolute blob part URLs.
export async function prepareFirmware(spec, baseURL, api = {}) {
  const fetchFile = api.fetch || globalThis.fetch;
  const digest = api.digest || ((bytes) => crypto.subtle.digest('SHA-256', bytes));
  const createURL = api.createURL || ((blob) => URL.createObjectURL(blob));
  const revokeURL = api.revokeURL || ((url) => URL.revokeObjectURL(url));
  const manifest = spec?.manifest;
  const build = manifest?.builds?.[0];
  const part = build?.parts?.[0];
  if (!/^[0-9a-f]{64}$/.test(spec?.sha256 || '') ||
      !/^manifest-[a-z0-9-]+-(update|neu)\.json$/.test(spec?.file || '') ||
      !/^[a-z0-9-]+\.bin$/.test(part?.path || '') ||
      manifest.builds.length !== 1 || build.chipFamily !== 'ESP32-S3' ||
      build.parts.length !== 1 || ![0, 0x10000].includes(part.offset) ||
      manifest.new_install_prompt_erase !== (part.offset === 0x10000) ||
      manifest.new_install_improv_wait_time !== 0) {
    throw new Error('Ungültige Firmware-Konfiguration.');
  }
  const base = new URL(baseURL);
  if (!['https:', 'http:'].includes(base.protocol)) throw new Error('Ungültige Download-Adresse.');
  const options = {cache: 'no-store', credentials: 'omit', redirect: 'error'};
  const response = await fetchFile(new URL(spec.file, base).href, options);
  if (!response.ok) throw new Error('Firmware-Manifest konnte nicht geladen werden.');
  const remote = await response.json();
  // Never allow a server manifest to substitute a target, file, offset or erase policy.
  if (remote?.name !== manifest.name || remote.version !== manifest.version ||
      remote.new_install_prompt_erase !== manifest.new_install_prompt_erase ||
      remote.new_install_improv_wait_time !== manifest.new_install_improv_wait_time ||
      remote.builds?.length !== 1 || remote.builds[0].chipFamily !== build.chipFamily ||
      remote.builds[0].parts?.length !== 1 ||
      remote.builds[0].parts[0].path !== part.path ||
      remote.builds[0].parts[0].offset !== part.offset) {
    throw new Error('Firmware-Manifest passt nicht zur Geräteauswahl.');
  }
  const download = await fetchFile(new URL(part.path, base).href, options);
  if (!download.ok) throw new Error('Firmware konnte nicht geladen werden.');
  const bytes = await download.arrayBuffer();
  const hash = Array.from(new Uint8Array(await digest(bytes)), b => b.toString(16).padStart(2, '0')).join('');
  if (hash !== spec.sha256) throw new Error('SHA-256 stimmt nicht. Installation gesperrt.');
  const header = new Uint8Array(bytes);
  if (header.length < 24 || header[0] !== 0xe9 || header[12] !== 9 || header[13] !== 0) {
    throw new Error('Firmware ist kein passendes ESP32-S3-Abbild.');
  }
  const urls = [];
  try {
    const firmwareURL = createURL(new Blob([bytes], {type: 'application/octet-stream'}));
    urls.push(firmwareURL);
    // Hand ESP Web Tools the verified immutable snapshot, never a second network download.
    const verified = {...manifest, builds: [{...build, parts: [{...part, path: firmwareURL}]}]};
    const manifestURL = createURL(new Blob([JSON.stringify(verified)], {type: 'application/json'}));
    urls.push(manifestURL);
    return {manifestURL, dispose: () => urls.splice(0).forEach(revokeURL)};
  } catch (error) {
    urls.forEach(revokeURL);
    throw error;
  }
}

export function mountFlasher(doc = document, win = window) {
  const specs = JSON.parse(doc.getElementById('flasher-integrity').textContent);
  const prepared = [];
  for (const button of doc.querySelectorAll('button[data-prepare]')) {
    const key = button.dataset.prepare;
    const installer = doc.querySelector(`esp-web-install-button[data-manifest="${key}"]`);
    const status = doc.getElementById(button.getAttribute('aria-describedby'));
    button.addEventListener('click', async () => {
      button.disabled = true;
      status.textContent = 'Firmware wird geladen und geprüft …';
      try {
        if (!win.isSecureContext || !win.crypto?.subtle) throw new Error('Die Prüfung braucht HTTPS.');
        const result = await prepareFirmware(specs[key], doc.baseURI);
        prepared.push(result);
        installer.setAttribute('manifest', result.manifestURL);
        installer.querySelector('button[slot="activate"]').disabled = false;
        installer.hidden = false;
        button.hidden = true;
        status.textContent = 'SHA-256 bestätigt. Jetzt Installation starten und USB-Port wählen.';
      } catch (error) {
        status.textContent = error.message || 'Prüfung fehlgeschlagen. Installation gesperrt.';
        button.disabled = false;
      }
    });
  }
  win.addEventListener('pagehide', event => {
    if (!event.persisted) prepared.splice(0).forEach(result => result.dispose());
  });
}
