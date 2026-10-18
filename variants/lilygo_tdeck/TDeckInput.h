#pragma once
// T-Deck Plus: eingebaute Tastatur (ESP32-C3, I2C 0x55) und Trackball (GPIO-Impulse).
// Lokale Testversion "Tippen am Geraet" (DisAHRsterRadio-MeshCore).
#include <Arduino.h>
#include <Wire.h>

#ifndef TDECK_KB_ADDR
  #define TDECK_KB_ADDR 0x55
#endif
#ifndef PIN_TB_UP
  #define PIN_TB_UP     3
#endif
#ifndef PIN_TB_DOWN
  #define PIN_TB_DOWN  15
#endif
#ifndef PIN_TB_LEFT
  #define PIN_TB_LEFT   1
#endif
#ifndef PIN_TB_RIGHT
  #define PIN_TB_RIGHT  2
#endif

// Rueckgabewerte von poll(): ASCII-Zeichen der Tastatur (0x08 Ruecktaste, 0x0D Enter)
// oder eine der folgenden Trackball-Richtungen.
#define TDECK_IN_UP     0x81
#define TDECK_IN_DOWN   0x82
#define TDECK_IN_LEFT   0x83
#define TDECK_IN_RIGHT  0x84

// Zaehler und Interrupt-Routinen liegen in target.cpp (IRAM, nicht inline).
extern volatile uint16_t tdeck_tb_up, tdeck_tb_down, tdeck_tb_left, tdeck_tb_right;
void tdeck_trackball_attach();

class TDeckInput {

  bool _started = false;
  bool _kb_found = false;
  unsigned long _next_kb_probe = 0, _next_kb_read = 0, _next_tb = 0;

  bool probeKeyboard() {
    Wire.beginTransmission(TDECK_KB_ADDR);
    return Wire.endTransmission() == 0;
  }

public:
  void begin() {
    if (_started) return;
    _started = true;
    pinMode(PIN_TB_UP, INPUT_PULLUP);
    pinMode(PIN_TB_DOWN, INPUT_PULLUP);
    pinMode(PIN_TB_LEFT, INPUT_PULLUP);
    pinMode(PIN_TB_RIGHT, INPUT_PULLUP);
    tdeck_trackball_attach();
    _next_kb_probe = millis() + 500;   // Tastatur-Controller bootet etwas spaeter
  }

  bool hasKeyboard() const { return _kb_found; }

  // Liefert hoechstens ein Ereignis pro Aufruf (0 = nichts).
  uint8_t poll() {
    if (!_started) return 0;
    unsigned long now = millis();

    if (now >= _next_tb) {
      noInterrupts();
      uint16_t u = tdeck_tb_up, d = tdeck_tb_down, l = tdeck_tb_left, r = tdeck_tb_right;
      tdeck_tb_up = tdeck_tb_down = tdeck_tb_left = tdeck_tb_right = 0;
      interrupts();
      uint16_t m = u; uint8_t ev = TDECK_IN_UP;
      if (d > m) { m = d; ev = TDECK_IN_DOWN; }
      if (l > m) { m = l; ev = TDECK_IN_LEFT; }
      if (r > m) { m = r; ev = TDECK_IN_RIGHT; }
      if (m > 0) { _next_tb = now + 120; return ev; }   // eine Bewegung je 120 ms
      _next_tb = now + 20;
    }

    if (!_kb_found) {
      if (now >= _next_kb_probe) {
        _kb_found = probeKeyboard();
        _next_kb_probe = now + 2000;
      }
      return 0;
    }
    if (now < _next_kb_read) return 0;
    _next_kb_read = now + 20;
    if (Wire.requestFrom((uint8_t)TDECK_KB_ADDR, (uint8_t)1) != 1) {
      _kb_found = false;   // spaeter erneut suchen
      _next_kb_probe = now + 2000;
      return 0;
    }
    int k = Wire.read();
    if (k <= 0) return 0;
    if (k == 0x7F) k = 0x08;   // manche Tastatur-Firmwares senden DEL statt Backspace
    if (k == 0x0A) k = 0x0D;
    if (k == 0x08 || k == 0x0D || (k >= 0x20 && k < 0x7F)) return (uint8_t)k;
    return 0;
  }
};
