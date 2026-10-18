#pragma once
// Nachrichten am Geraet tippen (T-Deck Plus: Tastatur + Trackball).
// Wird nur mit UI_HAS_KEYBOARD=1 gebaut und in UITask.cpp eingebunden.
//
// Ablauf: Startseite "Nachrichten" -> Enter -> Liste (Kanaele, dann Kontakte)
//         -> Trackball hoch/runter waehlen, Klick/Enter oeffnen
//         -> tippen, Enter sendet, Ruecktaste loescht, Trackball-Klick macht a/o/u/s zu ae/oe/ue/ss.

#ifndef CHAT_LOG_SIZE
  #define CHAT_LOG_SIZE 48
#endif
#define CHAT_INPUT_MAX   140   // Bytes (UTF-8)
#define CHAT_ENTRY_MAX   (CHAT_INPUT_MAX + 40)
#define CHAT_KEY_PREFIX  6
#define CHAT_MAX_TARGETS (MAX_GROUP_CHANNELS + MAX_CONTACTS)
#define CHAT_SEND_GAP_MS 3000

// UTF-8 -> Code Page 437 (Schrift des ST7789-Treibers): deutsche Umlaute bleiben lesbar.
static void chatToCP437(char* dest, const char* src, size_t dest_size) {
  size_t j = 0;
  for (size_t i = 0; src[i] != 0 && j < dest_size - 1; i++) {
    uint8_t c = (uint8_t)src[i];
    if (c >= 32 && c <= 126) { dest[j++] = c; continue; }
    if (c < 0x80) continue;   // Steuerzeichen weglassen
    char out = '?';
    if (c == 0xC3 && src[i+1]) {
      switch ((uint8_t)src[i+1]) {
        case 0xA4: out = (char)0x84; break;  // ae
        case 0xB6: out = (char)0x94; break;  // oe
        case 0xBC: out = (char)0x81; break;  // ue
        case 0x84: out = (char)0x8E; break;  // Ae
        case 0x96: out = (char)0x99; break;  // Oe
        case 0x9C: out = (char)0x9A; break;  // Ue
        case 0x9F: out = (char)0xE1; break;  // ss
        case 0xA9: out = (char)0x82; break;  // e acute
        case 0xA8: out = (char)0x8A; break;  // e grave
        case 0xA0: out = (char)0x85; break;  // a grave
        case 0xA7: out = (char)0x87; break;  // c cedille
      }
    } else if (c == 0xC2 && (uint8_t)src[i+1] == 0xB0) {
      out = (char)0xF8;  // Grad
    } else if (c == 0xE2 && (uint8_t)src[i+1] == 0x88 && (uint8_t)src[i+2] == 0x9A) {
      out = (char)0xFB;  // Haken (Wurzelzeichen)
    } else if (c == 0xE2 && (uint8_t)src[i+1] == 0x82 && (uint8_t)src[i+2] == 0xAC) {
      out = 'E';         // Euro
    }
    dest[j++] = out;
    while (src[i+1] && ((uint8_t)src[i+1] & 0xC0) == 0x80) i++;   // Folgebytes ueberspringen
  }
  dest[j] = 0;
}

class ChatScreen : public UIScreen {
  UITask* _task;
  NodePrefs* _prefs;

  struct Entry {
    uint8_t used;
    uint8_t kind;        // 0 = Kanal, 1 = Kontakt
    uint8_t ch_idx;
    uint8_t key[CHAT_KEY_PREFIX];
    uint8_t out;         // 1 = von mir gesendet
    uint8_t status;      // 0 -, 1 gesendet, 2 bestaetigt, 3 Fehler
    uint8_t seen;
    uint32_t ack;
    char text[CHAT_ENTRY_MAX];
  };
  Entry _log[CHAT_LOG_SIZE];
  int _log_head = 0;     // naechster freier Platz

  // Zielliste
  uint8_t _ch_list[MAX_GROUP_CHANNELS];
  int _num_ch = 0;
  uint16_t _ct_list[MAX_CONTACTS];
  int _num_ct = 0;
  int _sel = 0, _list_top = 0;

  // Offenes Gespraech
  bool _in_chat = false;
  uint8_t _kind = 0, _ch_idx = 0;
  uint8_t _key[CHAT_KEY_PREFIX];
  char _title[34];
  char _input[CHAT_INPUT_MAX + 1];
  int _input_len = 0;
  int _scroll = 0;       // Zeilen nach oben gescrollt
  unsigned long _last_send = 0;
  bool _sent_once = false;

  // Zeilenpuffer fuer das Rendern
  #define CHAT_MAX_LINES 64
  #define CHAT_LINE_CHARS 40
  char _lines[CHAT_MAX_LINES][CHAT_LINE_CHARS];
  uint8_t _line_out[CHAT_MAX_LINES];

  int targetCount() const { return _num_ch + _num_ct; }

  void rebuildTargets() {
    _num_ch = 0;
    for (int i = 0; i < MAX_GROUP_CHANNELS; i++) {
      ChannelDetails ch;
      if (the_mesh.getChannel(i, ch) && ch.name[0]) _ch_list[_num_ch++] = i;
    }
    _num_ct = 0;
    int n = the_mesh.getNumContacts();
    for (int i = 0; i < n && _num_ct < MAX_CONTACTS; i++) {
      ContactInfo c;
      if (the_mesh.getContactByIdx(i, c) && c.type == ADV_TYPE_CHAT) _ct_list[_num_ct++] = i;
    }
    if (_sel >= targetCount()) _sel = targetCount() > 0 ? targetCount() - 1 : 0;
  }

  bool matches(const Entry& e, uint8_t kind, uint8_t ch_idx, const uint8_t* key) const {
    if (!e.used || e.kind != kind) return false;
    if (kind == 0) return e.ch_idx == ch_idx;
    return memcmp(e.key, key, CHAT_KEY_PREFIX) == 0;
  }

  bool hasUnseen(uint8_t kind, uint8_t ch_idx, const uint8_t* key) const {
    for (int i = 0; i < CHAT_LOG_SIZE; i++)
      if (matches(_log[i], kind, ch_idx, key) && !_log[i].seen) return true;
    return false;
  }

  void markSeen() {
    for (int i = 0; i < CHAT_LOG_SIZE; i++)
      if (matches(_log[i], _kind, _ch_idx, _key)) _log[i].seen = 1;
  }

  Entry* addEntry(uint8_t kind, uint8_t ch_idx, const uint8_t* key, bool out, const char* text) {
    Entry* e = &_log[_log_head];
    _log_head = (_log_head + 1) % CHAT_LOG_SIZE;
    memset(e, 0, sizeof(*e));
    e->used = 1; e->kind = kind; e->ch_idx = ch_idx; e->out = out ? 1 : 0;
    if (key) memcpy(e->key, key, CHAT_KEY_PREFIX);
    StrHelper::strncpy(e->text, text, sizeof(e->text));
    e->seen = (out || (_in_chat && _task->isChatOpen() && matches(*e, _kind, _ch_idx, _key))) ? 1 : 0;
    return e;
  }

  bool targetAt(int idx, uint8_t& kind, uint8_t& ch_idx, uint8_t* key, char* name, size_t name_sz) {
    if (idx < _num_ch) {
      ChannelDetails ch;
      if (!the_mesh.getChannel(_ch_list[idx], ch)) return false;
      kind = 0; ch_idx = _ch_list[idx];
      StrHelper::strncpy(name, ch.name, name_sz);
      return true;
    }
    idx -= _num_ch;
    if (idx < 0 || idx >= _num_ct) return false;
    ContactInfo c;
    if (!the_mesh.getContactByIdx(_ct_list[idx], c)) return false;
    kind = 1; ch_idx = 0;
    memcpy(key, c.id.pub_key, CHAT_KEY_PREFIX);
    StrHelper::strncpy(name, c.name, name_sz);
    return true;
  }

  void openSelected() {
    char name[32];
    if (!targetAt(_sel, _kind, _ch_idx, _key, name, sizeof(name))) return;
    snprintf(_title, sizeof(_title), "%s%s", _kind == 0 ? "#" : "@", name);
    _in_chat = true;
    _input_len = 0; _input[0] = 0;
    _scroll = 0;
    markSeen();
  }

  int utf8Back(int len) const {   // Laenge ohne das letzte UTF-8-Zeichen
    if (len <= 0) return 0;
    len--;
    while (len > 0 && (((uint8_t)_input[len]) & 0xC0) == 0x80) len--;
    return len;
  }

  void appendChar(char k) {
    if (_input_len + 1 > maxInput()) { _task->showAlert("Nachricht voll", 800); return; }
    _input[_input_len++] = k; _input[_input_len] = 0;
  }

  int maxInput() const {
    if (_kind == 0) {   // Kanal: "Name: Text" muss ins Paket passen
      int m = MAX_TEXT_LEN - 2 - (int)strlen(_prefs->node_name);
      return m < CHAT_INPUT_MAX ? m : CHAT_INPUT_MAX;
    }
    return CHAT_INPUT_MAX;
  }

  // Trackball-Klick: letztes a/o/u/s/A/O/U in Umlaut wandeln (und zurueck).
  void toggleUmlaut() {
    static const char* const plain = "aouAOUs";
    static const uint8_t uml[] = {0xA4, 0xB6, 0xBC, 0x84, 0x96, 0x9C, 0x9F};
    if (_input_len <= 0) return;
    uint8_t last = (uint8_t)_input[_input_len - 1];
    if (_input_len >= 2 && (uint8_t)_input[_input_len - 2] == 0xC3) {
      for (int i = 0; i < 7; i++) if (uml[i] == last) {
        _input_len -= 2; _input[_input_len++] = plain[i]; _input[_input_len] = 0; return;
      }
    }
    for (int i = 0; i < 7; i++) if ((uint8_t)plain[i] == last) {
      if (_input_len + 1 > maxInput()) return;
      _input_len--;
      _input[_input_len++] = (char)0xC3; _input[_input_len++] = (char)uml[i]; _input[_input_len] = 0;
      return;
    }
  }

  void sendInput() {
    if (_input_len == 0) return;
    if (_sent_once && millis() - _last_send < CHAT_SEND_GAP_MS) { _task->showAlert("Kurz warten...", 800); return; }
    uint32_t ts = rtc_clock.getCurrentTimeUnique();
    bool ok = false;
    uint32_t ack = 0;
    if (_kind == 0) {
      ChannelDetails ch;
      ok = the_mesh.getChannel(_ch_idx, ch) && the_mesh.sendGroupMessage(ts, ch.channel, _prefs->node_name, _input, _input_len);
    } else {
      ContactInfo* r = the_mesh.lookupContactByPubKey(_key, CHAT_KEY_PREFIX);
      if (r) {
        uint32_t est_timeout;
        ok = the_mesh.sendMessage(*r, ts, 0, _input, ack, est_timeout) != MSG_SEND_FAILED;
      }
    }
    if (!ok) {   // Text bleibt im Eingabefeld, damit man es erneut versuchen kann
      _task->showAlert("Senden fehlgeschlagen", 1200);
      return;
    }
    Entry* e = addEntry(_kind, _ch_idx, _key, true, _input);
    e->status = 1;
    e->ack = ack;
    _task->showAlert("Gesendet", 700);
    _input_len = 0; _input[0] = 0;
    _last_send = millis(); _sent_once = true;
    _scroll = 0;
  }

  // Zeilenumbruch nach Zeichen (nach Leerzeichen, wenn moeglich).
  int wrapInto(const char* cp, int cols, uint8_t out, int nlines) {
    int len = strlen(cp), pos = 0;
    while (pos < len && nlines < CHAT_MAX_LINES) {
      int take = len - pos;
      if (take > cols) {
        take = cols;
        for (int k = cols; k > cols / 2; k--) if (cp[pos + k] == ' ') { take = k; break; }
      }
      if (take >= CHAT_LINE_CHARS) take = CHAT_LINE_CHARS - 1;
      memcpy(_lines[nlines], cp + pos, take); _lines[nlines][take] = 0;
      _line_out[nlines] = out;
      nlines++;
      pos += take;
      while (pos < len && cp[pos] == ' ') pos++;
    }
    return nlines;
  }

  int columns(DisplayDriver& display) {
    int w10 = display.getTextWidth("MMMMMMMMMM");
    if (w10 <= 0) return 20;
    int cols = (display.width() * 10) / w10;
    if (cols >= CHAT_LINE_CHARS) cols = CHAT_LINE_CHARS - 1;
    return cols < 8 ? 8 : cols;
  }

  void renderList(DisplayDriver& display) {
    display.setColor(UIColor::title_bkg);
    display.fillRect(0, 0, display.width(), 6);
    display.setColor(UIColor::title_txt);
    display.setCursor(1, 1);
    display.print("Nachricht an ...");
    const int row_h = 5, top = 7, rows = 10;
    int n = targetCount();
    if (n == 0) {
      display.setColor(UIColor::secondary_txt);
      display.setCursor(1, top + 5);  display.print("Keine Kanaele/Kontakte.");
      display.setCursor(1, top + 10); display.print("Links: zurueck");
      return;
    }
    if (_sel < _list_top) _list_top = _sel;
    if (_sel >= _list_top + rows) _list_top = _sel - rows + 1;
    for (int r = 0; r < rows && _list_top + r < n; r++) {
      int idx = _list_top + r;
      uint8_t kind, ch_idx, key[CHAT_KEY_PREFIX]; char name[32];
      if (!targetAt(idx, kind, ch_idx, key, name, sizeof(name))) continue;
      char line[48], cp[48];
      snprintf(line, sizeof(line), "%s%s%s", hasUnseen(kind, ch_idx, key) ? "*" : " ", kind == 0 ? "#" : "@", name);
      chatToCP437(cp, line, sizeof(cp));
      int y = top + r * row_h;
      if (idx == _sel) {
        display.setColor(UIColor::corp_blue);
        display.fillRect(0, y, display.width(), row_h);
        display.setColor(UIColor::title_txt);
      } else {
        display.setColor(kind == 0 ? UIColor::corp_blue : UIColor::primary_txt);
      }
      display.setCursor(1, y);
      display.print(cp);
    }
    display.setColor(UIColor::secondary_txt);
    display.setCursor(1, 59);
    display.print("Klick/Enter: oeffnen  Links: zurueck");
  }

  void renderChat(DisplayDriver& display) {
    display.setColor(UIColor::title_bkg);
    display.fillRect(0, 0, display.width(), 6);
    display.setColor(UIColor::title_txt);
    char cp[CHAT_ENTRY_MAX + 48];
    chatToCP437(cp, _title, sizeof(cp));
    display.setCursor(1, 1);
    display.print(cp);

    int cols = columns(display);
    // Zeilen fuer dieses Gespraech, aelteste zuerst
    int nlines = 0;
    for (int k = 0; k < CHAT_LOG_SIZE; k++) {
      const Entry& e = _log[(_log_head + k) % CHAT_LOG_SIZE];
      if (!matches(e, _kind, _ch_idx, _key)) continue;
      char line[CHAT_ENTRY_MAX + 48];
      if (e.out) {
        const char* st = e.status == 2 ? " \xE2\x88\x9A" : (e.status == 3 ? " !" : "");
        snprintf(line, sizeof(line), "Ich: %s%s", e.text, st);
      } else {
        StrHelper::strncpy(line, e.text, sizeof(line));
      }
      chatToCP437(cp, line, sizeof(cp));
      nlines = wrapInto(cp, cols, e.out, nlines);
      if (nlines >= CHAT_MAX_LINES) {   // aelteste Zeilen verwerfen
        memmove(_lines[0], _lines[16], (CHAT_MAX_LINES - 16) * CHAT_LINE_CHARS);
        memmove(_line_out, _line_out + 16, CHAT_MAX_LINES - 16);
        nlines -= 16;
      }
    }
    const int row_h = 5, top = 7, rows = 9;   // Verlauf: y 7..51
    int max_scroll = nlines > rows ? nlines - rows : 0;
    if (_scroll > max_scroll) _scroll = max_scroll;
    int first = nlines - rows - _scroll; if (first < 0) first = 0;
    if (nlines == 0) {
      display.setColor(UIColor::secondary_txt);
      display.setCursor(1, top);
      display.print("Noch keine Nachrichten.");
    }
    for (int r = 0; r < rows && first + r < nlines; r++) {
      display.setColor(_line_out[first + r] ? UIColor::corp_blue : UIColor::primary_txt);
      display.setCursor(1, top + r * row_h);
      display.print(_lines[first + r]);
    }
    if (_scroll > 0) {
      display.setColor(UIColor::warning_txt);
      display.drawTextRightAlign(display.width() - 1, top, "^");
    }

    // Eingabe (zwei Zeilen, Ende sichtbar)
    display.setColor(UIColor::secondary_txt);
    display.fillRect(0, 52, display.width(), 1);
    char shown[CHAT_INPUT_MAX + 8];
    snprintf(shown, sizeof(shown), ">%s_", _input);
    chatToCP437(cp, shown, sizeof(cp));
    int len = strlen(cp);
    int start = len > cols * 2 ? len - cols * 2 : 0;
    display.setColor(UIColor::primary_txt);
    for (int r = 0; r < 2; r++) {
      int off = start + r * cols;
      if (off >= len) break;
      char seg[CHAT_LINE_CHARS];
      int take = len - off; if (take > cols) take = cols;
      memcpy(seg, cp + off, take); seg[take] = 0;
      display.setCursor(1, 54 + r * row_h);
      display.print(seg);
    }
  }

public:
  ChatScreen(UITask* task, NodePrefs* prefs) : _task(task), _prefs(prefs) {
    memset(_log, 0, sizeof(_log));
    _input[0] = 0; _title[0] = 0;
  }

  void open() {
    rebuildTargets();
    _in_chat = false;
  }

  bool composing() const { return _in_chat && _input_len > 0; }

  void onContactMsg(const ContactInfo& from, const char* text) {
    char line[CHAT_ENTRY_MAX];
    char nm[14]; StrHelper::strncpy(nm, from.name, sizeof(nm));
    snprintf(line, sizeof(line), "%s: %s", nm, text);
    addEntry(1, 0, from.id.pub_key, false, line);
  }

  void onChannelMsg(const ChannelDetails& ch, const char* text) {
    int idx = the_mesh.findChannelIdx(ch.channel);
    if (idx < 0) return;
    addEntry(0, (uint8_t)idx, NULL, false, text);   // Text enthaelt schon "Absender: ..."
  }

  void onAck(uint32_t ack) {
    if (ack == 0) return;
    for (int i = 0; i < CHAT_LOG_SIZE; i++)
      if (_log[i].used && _log[i].out && _log[i].ack == ack) { _log[i].status = 2; _log[i].ack = 0; }
  }

  bool isShowing(uint8_t kind, uint8_t ch_idx, const uint8_t* key) const {
    if (!_in_chat) return false;
    if (kind != _kind) return false;
    return kind == 0 ? ch_idx == _ch_idx : memcmp(key, _key, CHAT_KEY_PREFIX) == 0;
  }

  int render(DisplayDriver& display) override {
    display.setTextSize(1);
    if (_in_chat) renderChat(display); else renderList(display);
    return 1000;
  }

  bool handleInput(char ch) override {
    uint8_t c = (uint8_t)ch;
    if (!_in_chat) {
      int n = targetCount();
      if (c == TDECK_IN_UP || c == (uint8_t)KEY_PREV) { if (_sel > 0) _sel--; return true; }
      if (c == TDECK_IN_DOWN) { if (_sel + 1 < n) _sel++; return true; }
      if (c == TDECK_IN_LEFT || c == 0x08) { _task->gotoHomeScreen(); return true; }
      if ((c == (uint8_t)KEY_NEXT || c == (uint8_t)KEY_ENTER || c == TDECK_IN_RIGHT) && n > 0) { openSelected(); return true; }
      return false;
    }
    if (c == TDECK_IN_UP)   { _scroll++; return true; }
    if (c == TDECK_IN_DOWN) { if (_scroll > 0) _scroll--; return true; }
    if (c == TDECK_IN_LEFT && _input_len == 0) { _in_chat = false; rebuildTargets(); return true; }
    if (c == (uint8_t)KEY_NEXT) { toggleUmlaut(); return true; }       // Trackball-Klick
    if (c == (uint8_t)KEY_PREV) { return true; }                        // Doppelklick: ignorieren
    if (c == 0x08) {
      if (_input_len == 0) { _in_chat = false; rebuildTargets(); }
      else { _input_len = utf8Back(_input_len); _input[_input_len] = 0; }
      return true;
    }
    if (c == (uint8_t)KEY_ENTER) { sendInput(); return true; }
    if (c >= 0x20 && c < 0x7F) { appendChar((char)c); _scroll = 0; return true; }
    return false;
  }
};
