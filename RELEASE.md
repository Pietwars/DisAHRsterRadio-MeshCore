# Firmware veröffentlichen

Aktuell: **0.9.4**, MeshCore 1.18.0, LILYGO T-Deck Plus und LILYGO T-Watch S3.
Die ursprüngliche MeshCore-Anleitung steht in [README.meshcore.md](README.meshcore.md).

**Getrennt vom stabilen 0.9.4-Release:** Das experimentelle [T-Deck-Standalone-Prerelease `standalone-test5`](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/releases/tag/standalone-test5) ist veröffentlicht, aber **nicht auf echter Hardware abgenommen** und nicht im Web-Flasher. Hardwareabnahme: [Protokoll](docs/STANDALONE-ABNAHME.md) / [Issue #40](https://github.com/Pietwars/DisAHRsterRadio-MeshCore/issues/40). Ein stabiler Standalone-Tag setzt dokumentierte Gerätetests und eine separate Freigabeentscheidung voraus.

1. `VERSION` ändern, `python3 tools/version.py` und `python3 tools/embed_web.py`
   ausführen. README, CHANGELOG, Geräteanleitung und Hardwarestatus aktualisieren.
2. Hosttests und beide Board-Builds prüfen; Flash-/Speicherprüfung ist im Build
   und in `tools/build_device.sh` integriert. Details: [SPEICHER.md](docs/SPEICHER.md).
3. Den geprüften Commit auf `main` mit `v<VERSION>` taggen und den Tag pushen.
   Der Workflow `release.yml` baut beide Geräte, prüft die ZIPs und veröffentlicht
   sie mit SHA256SUMS und Release-Text. Tags mit Bindestrich werden als Prerelease
   veröffentlicht. Vorhandene Releases werden nicht überschrieben.
4. Veröffentlichte ZIPs herunterladen und erneut mit `tools/verify_devices.py`
   prüfen. Mit `tools/flasher-next/aktualisieren.py` die SHA256-Werte aus genau
   diesen ZIPs übernehmen, Konfiguration prüfen und nach `docs/flasher/` kopieren.
   Das Release muss vor der Aktivierung im Web-Flasher verfügbar sein.
5. Pages-Lauf und vier Flasher-Manifeste prüfen: Update nur an `0x10000`,
   Neuinstallation an `0x0`. Ein Update schreibt weder SPIFFS noch NVS.

Hardware-Rückmeldungen und noch offene Tests werden ausdrücklich dokumentiert;
ein erfolgreicher Compiler-Build ist keine vollständige Hardwareabnahme.
