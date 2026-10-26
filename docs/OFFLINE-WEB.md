# Offline-Webvorschau

Teilaufgabe „PWA Offline-Cache“ aus #47. Die auf HTTPS/GitHub Pages bereitgestellte
Demo speichert ihre Oberfläche, Hintergrund, Symbol, Manifest und alle fünf
mitgelieferten Karten nach dem ersten vollständigen Online-Aufruf.
Die Karten liegen komprimiert und entpackt im Cache, damit beide vorhandenen
Ladewege offline funktionieren. Es werden keine zusätzlichen Kartendienste
angesprochen.

Die Anzeige unter Info unterscheidet „Offline-Vorschau bereit“ von Offlinebetrieb
und fehlgeschlagener Vorbereitung. Es bleiben ausdrücklich simulierte Daten.
Das Schließen/Öffnen oder Neuladen der vorbereiteten Demo funktioniert ohne
Internet. Ein erster Aufruf ohne vorherige erfolgreiche Vorbereitung funktioniert
nicht. Browser können gespeicherte Daten entfernen; dies ist kein dauerhaftes
Backup. Es gibt keine neue Garantie für automatische App-Installationsangebote
auf allen Mobilplattformen.

## Grenzen und Datenschutz

- Registrierung nur im Demo-Modus und in einem sicheren Browserkontext.
- Das Funkgerät unter `http://192.168.4.1` bleibt ohne Service Worker. Seine
  Oberfläche und Karten werden wie bisher direkt vom Gerät geliefert; dafür
  braucht es kein Internet. Diese Änderung macht eine HTTPS-Demo nicht zu einer
  fernverbundenen Funkgeräte-App.
- Keine Geräte-APIs, privaten Nachrichten, POST-Anfragen oder Sendewarteschlangen
  im Browsercache. Keine nachträgliche Übertragung vermeintlich offline gesendeter
  Nachrichten. Firmware, Webflasher und andere Seiten werden nicht mitgespeichert.
- Cachezugriff nur auf explizit aufgeführte Dateien im eigenen Ursprung und Pfad.
  Andere Anwendungen und andere Installationspfade behalten ihre Caches.

## Aktualisierung

`tools/build_pages.py` berechnet aus Oberfläche, Karten, Ressourcen und Worker-Code
einen Fingerabdruck. Ein neuer Cache wird vollständig vorbereitet, bevor sein
Worker aktiv werden kann. Ein fehlgeschlagener Download lässt die neue
Installation scheitern. Ein vorhandener aktiver Worker bleibt bestehen.
Ein wartendes Update übernimmt nach dem Schließen der bisherigen Demo-Fenster;
kein erzwungener Wechsel während der Benutzung. Alte Caches desselben Pfads
werden erst bei Aktivierung des Nachfolgers entfernt.

## Prüfung

`node tests/offline_shell.test.cjs`: fehlgeschlagene Installation, explizite
Dateiliste, Offlineantworten, getrennte Installationspfade und ausgeschlossene
API-/POST-/Firmware-Anfragen.

`node tests/offline_browser.test.cjs`: echte Chromium-Service-Worker-Registrierung
unter einem Unterpfad, Offline-Neuladen, alle fünf Karten und fehlschlagender
Geräte-API-Zugriff statt Rückgabe eines alten Caches. Verwendet die erzeugte
Pages-Demo und keinen echten Funkknoten. Lokale Chromium-Binärdatei fehlt in der
Arbeitsumgebung; dieser Browsernachweis wird in CI geprüft.
