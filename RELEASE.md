# Firmware veröffentlichen

Aktuell: **0.9.6**, MeshCore 1.18.0. Regulärer öffentlicher Release mit getrennten Paketen für T-Deck Plus WLAN/BLE, T-Watch S3 WLAN/BLE und T-Deck Plus Standalone. Die technische Standalone-Kennung `standalone-test6` bleibt bestehen.

**Aktueller Entwicklungsstand:** `update/096-current` integriert SOS und
DE-Sendeschutz auf Basis 0.9.6. Der gemeinsame Stand muss seine eigenen Tests,
Firmware-Builds und Geräteabnahmen erhalten. Die bisherigen Releasepakete,
Tags und Flasher-Prüfsummen bleiben unverändert. Vor einer neuen Veröffentlichung
ist eine eindeutige neue Releasekennung festzulegen; 0.9.6 nicht überschreiben.
Details: [Release und Entwicklungsstand](docs/RELEASE-0.9.6.md).

1. `VERSION` ändern, `python3 tools/version.py` und `python3 tools/embed_web.py` ausführen.
2. Host-, Browser-, Firmware-, Flash-/Speicher- und Paketprüfungen müssen bestehen. Der Release-Workflow prüft alle drei Pakete und die aktuellen Demo-Vorschauen.
3. Der geprüfte Stand wird auf `main` übernommen. Bei einer Versionsänderung erstellt der Release-Workflow den Tag und veröffentlicht die Pakete mit SHA256SUMS. Tags mit Bindestrich sind Pre-Releases. Vorhandene Firmware-Pakete werden nicht überschrieben.
4. Veröffentlichte ZIPs herunterladen, die äußeren und inneren Prüfsummen sowie die Paketstruktur prüfen. Erst aus diesen Bytes `tools/flasher-next/aktualisieren.py` ausführen und die geprüfte Konfiguration nach `docs/flasher/` übernehmen.
5. Erfolgreiche Pages-Bereitstellung und **sechs** Flash-Manifeste prüfen: drei Varianten, jeweils Update an `0x10000` und Neuinstallation an `0x0`. Ein Update schreibt weder SPIFFS noch NVS; Neuinstallation löscht Gerätedaten.

Die korrigierte [Standalone-Bedienung](docs/STANDALONE_TOUCH.md) wird zusätzlich zum unveränderten ursprünglichen 0.9.6-ZIP beim Release bereitgestellt. Der frühere Hinweis „Kein Release, kein Merge“ im Kopf der dort enthaltenen BEDIENUNG.md ist überholt.

Hardware-Rückmeldungen und offene Prüfungen stehen in [RELEASE-0.9.6.md](docs/RELEASE-0.9.6.md). Erfolgreiche Builds ersetzen keine vollständige Hardwareabnahme.
