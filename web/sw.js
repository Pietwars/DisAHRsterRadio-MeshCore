// Static demo shell only. Never cache device APIs, messages, firmware or flasher files.
const VERSION = 'development'; // Replaced by tools/build_pages.py with an asset fingerprint.
const ROOT = new URL('./', self.registration.scope);
const PREFIX = 'dar-demo-shell:' + ROOT.href + ':';
const CACHE = PREFIX + VERSION;
const FILES = ['index.html', 'app.webmanifest', 'app-icon.png', 'bg.webp',
  'map/ahrtal.json.gz', 'map/altenahr.json.gz', 'map/dernau.json.gz',
  'map/heimersheim.json.gz', 'map/sinzig.json.gz',
  'map/ahrtal.json', 'map/altenahr.json', 'map/dernau.json',
  'map/heimersheim.json', 'map/sinzig.json'];
const URLS = new Set(FILES.map(path => new URL(path, ROOT).href));
self.addEventListener('install', event => {
  // addAll rejects failed downloads. A partial install must never become active.
  event.waitUntil(caches.open(CACHE).then(cache => cache.addAll([...URLS].map(url => new Request(url, {cache: 'reload', credentials: 'omit'})))));
});
self.addEventListener('activate', event => {
  event.waitUntil((async () => {
    for (const name of await caches.keys()) {
      if (name.startsWith(PREFIX) && name !== CACHE) await caches.delete(name);
    }
    await self.clients.claim();
  })());
});
self.addEventListener('fetch', event => {
  const request = event.request;
  const url = new URL(request.url);
  if (request.method !== 'GET' || url.origin !== ROOT.origin) return;
  const shell = request.mode === 'navigate' &&
    (url.pathname === ROOT.pathname || url.pathname === new URL('index.html', ROOT).pathname);
  if (shell) {
    event.respondWith((async () => {
      // Keep each active worker's complete shell/assets on the same fingerprint.
      const cache = await caches.open(CACHE);
      return (await cache.match(new URL('index.html', ROOT).href)) || fetch(request);
    })());
  } else if (URLS.has(url.href)) {
    event.respondWith(caches.open(CACHE).then(async cache => (await cache.match(request)) || fetch(request)));
  }
});
