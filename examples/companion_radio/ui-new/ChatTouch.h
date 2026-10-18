// Touch-Bedienung fuer ChatScreen (T-Watch S3, 240x240 als 128x128 logisch).
// Wird INNERHALB der Klasse ChatScreen eingebunden (Member-Funktionen).

  enum TouchMode : uint8_t { T_LIST, T_CHAT, T_QUICK, T_KEYS };
  uint8_t _tmode = T_LIST;
  uint8_t _kpage = 0;      // 0 Buchstaben, 1 Ziffern/Zeichen
  bool _shift = false;

  static const char* const* quickReplies(int& n) {
    static const char* const q[] = {
      "OK", "Brauche Hilfe", "Bin sicher", "Bin unterwegs", "Wo bist du?", "Danke", "Nein"
    };
    n = sizeof(q) / sizeof(q[0]);
    return q;
  }

  // Tastaturreihen (UTF-8); Reihe 3 ist die Sondertasten-Reihe.
  const char* keyLabel(int row, int col) const {
    static const char* const lower[3][10] = {
      {"q","w","e","r","t","z","u","i","o","p"},
      {"a","s","d","f","g","h","j","k","l","\xC3\x9F"},
      {"y","x","c","v","b","n","m","\xC3\xA4","\xC3\xB6","\xC3\xBC"}};
    static const char* const upper[3][10] = {
      {"Q","W","E","R","T","Z","U","I","O","P"},
      {"A","S","D","F","G","H","J","K","L","\xC3\x9F"},
      {"Y","X","C","V","B","N","M","\xC3\x84","\xC3\x96","\xC3\x9C"}};
    static const char* const sym[3][10] = {
      {"1","2","3","4","5","6","7","8","9","0"},
      {".",",","?","!","-",":",";","/","(",")"},
      {"@","#","+","=","*","'","\"","%","&","_"}};
    if (row < 0 || row > 2 || col < 0 || col > 9) return "";
    if (_kpage == 1) return sym[row][col];
    return _shift ? upper[row][col] : lower[row][col];
  }

  // Sondertasten: Breiten in Zehntel der Displaybreite
  //   Umschalt 1.2 | 123/abc 1.6 | Leer 2.8 | Loeschen 1.4 | Senden 3.0
  int specialKeyAt(int x, int w) const {
    static const int edges[] = {12, 28, 56, 70, 100};
    int p = (x * 100) / w;
    for (int i = 0; i < 5; i++) if (p < edges[i]) return i;
    return 4;
  }

  int keysTop(DisplayDriver& d) const { return d.height() - 5 * keyH(d); }
  int keyH(DisplayDriver& d) const { return d.height() / 6 - 1; }   // 128 -> 20

  void drawKey(DisplayDriver& d, int x, int y, int w, int h, const char* utf8, bool hi) {
    char cp[16];
    chatToCP437(cp, utf8, sizeof(cp));
    if (hi) { d.setColor(UIColor::corp_blue); d.fillRect(x + 1, y + 1, w - 2, h - 2); }
    d.setColor(UIColor::secondary_txt);
    d.drawRect(x, y, w, h);
    d.setColor(hi ? UIColor::title_txt : UIColor::primary_txt);
    d.drawTextCentered(x + w / 2, y + (h - 8) / 2, cp);
  }

  void renderButtons(DisplayDriver& d, const char* a, const char* b, const char* c) {
    int y = d.height() - 2 * CHAT_ROW_H, h = 2 * CHAT_ROW_H, w = d.width() / 3;
    drawKey(d, 0, y, w, h, a, false);
    drawKey(d, w, y, w, h, b, false);
    drawKey(d, 2 * w, y, d.width() - 2 * w, h, c, true);
  }

  void renderTouchList(DisplayDriver& d) {
    renderHeader(d, "Nachricht an ...");
    const int top = CHAT_ROW_H + 2, bottom = d.height() - 2 * CHAT_ROW_H - 1;
    const int rows = (bottom - top) / CHAT_ROW_H;
    int n = targetCount();
    if (_list_top > n - rows) _list_top = n - rows;
    if (_list_top < 0) _list_top = 0;
    if (n == 0) {
      d.setColor(UIColor::secondary_txt);
      d.setCursor(1, top + CHAT_ROW_H);
      d.print("Keine Kanaele/Kontakte.");
    }
    for (int r = 0; r < rows && _list_top + r < n; r++) {
      uint8_t kind, ch_idx, key[CHAT_KEY_PREFIX]; char name[32], line[48], cp[48];
      if (!targetAt(_list_top + r, kind, ch_idx, key, name, sizeof(name))) continue;
      snprintf(line, sizeof(line), "%s%s%s", hasUnseen(kind, ch_idx, key) ? "*" : " ", kind == 0 ? "#" : "@", name);
      chatToCP437(cp, line, sizeof(cp));
      d.setColor(kind == 0 ? UIColor::corp_blue : UIColor::primary_txt);
      d.setCursor(1, top + r * CHAT_ROW_H);
      d.print(cp);
      d.setColor(UIColor::secondary_txt);
      d.fillRect(0, top + (r + 1) * CHAT_ROW_H - 1, d.width(), 1);
    }
    renderButtons(d, "hoch", "runter", "zurueck");
  }

  void renderTouchChat(DisplayDriver& d) {
    renderHeader(d, _title);
    renderHistory(d, CHAT_ROW_H + 2, d.height() - 2 * CHAT_ROW_H - 1);
    renderButtons(d, "zurueck", "schnell", "tippen");
  }

  void renderQuick(DisplayDriver& d) {
    renderHeader(d, "Schnellantwort");
    int n; const char* const* q = quickReplies(n);
    int top = CHAT_ROW_H + 2, h = (d.height() - top) / (n + 1);
    for (int i = 0; i < n; i++) drawKey(d, 0, top + i * h, d.width(), h, q[i], false);
    drawKey(d, 0, top + n * h, d.width(), h, "abbrechen", true);
  }

  void renderKeys(DisplayDriver& d) {
    int kh = keyH(d), kt = keysTop(d), w = d.width();
    // Eingabezeile(n) oben, rechts "X" zum Schliessen
    renderInput(d, 1, w - 18);
    drawKey(d, w - 16, 0, 16, kt - 2, "X", false);
    for (int r = 0; r < 3; r++)
      for (int c = 0; c < 10; c++) {
        int x0 = (c * w) / 10, x1 = ((c + 1) * w) / 10;
        drawKey(d, x0, kt + r * kh, x1 - x0, kh, keyLabel(r, c), false);
      }
    static const int edges[] = {0, 12, 28, 56, 70, 100};
    const char* lab[5] = {"^", _kpage ? "abc" : "123", "Leer", "<-", "Senden"};
    int y = kt + 3 * kh;
    for (int i = 0; i < 5; i++) {
      int x0 = (edges[i] * w) / 100, x1 = (edges[i + 1] * w) / 100;
      drawKey(d, x0, y, x1 - x0, d.height() - y, lab[i], (i == 0 && _shift) || i == 4);
    }
  }

  void typeKey(const char* utf8) {
    int l = strlen(utf8);
    if (_input_len + l > maxInput()) { _task->showAlert("Nachricht voll", 800); return; }
    memcpy(_input + _input_len, utf8, l);
    _input_len += l; _input[_input_len] = 0;
    _shift = false;
  }

  void sendQuick(const char* text) {
    char saved[CHAT_INPUT_MAX + 1]; int saved_len = _input_len;
    memcpy(saved, _input, _input_len + 1);
    StrHelper::strncpy(_input, text, sizeof(_input));
    _input_len = strlen(_input);
    sendInput();
    if (_input_len == 0) {      // gesendet: Entwurf wiederherstellen
      memcpy(_input, saved, saved_len + 1); _input_len = saved_len;
    } else {                    // fehlgeschlagen
      memcpy(_input, saved, saved_len + 1); _input_len = saved_len;
    }
  }

public:
  int render(DisplayDriver& display) override {
    display.setTextSize(1);
    if (!_in_chat) _tmode = T_LIST;
    else if (_tmode == T_LIST) _tmode = T_CHAT;
    switch (_tmode) {
      case T_LIST:  renderTouchList(display); break;
      case T_CHAT:  renderTouchChat(display); break;
      case T_QUICK: renderQuick(display); break;
      default:      renderKeys(display); break;
    }
    return 1000;
  }

  // Tasten aus der normalen Touch-Zuordnung (falls doch welche ankommen) ignorieren.
  bool handleInput(char ch) override { return false; }

  // Ein abgeschlossener Touch: Start (x0,y0), Ende (x1,y1), Haltedauer.
  void onTouch(DisplayDriver& d, int x0, int y0, int x1, int y1, unsigned long held) {
    int dy = y1 - y0;
    bool swipe = dy > 12 || dy < -12;
    int btn_y = d.height() - 2 * CHAT_ROW_H, w = d.width();
    const int top = CHAT_ROW_H + 2;
    if (!_in_chat) _tmode = T_LIST;

    if (_tmode == T_LIST) {
      const int rows = (btn_y - 1 - top) / CHAT_ROW_H;
      if (swipe) { _list_top += dy < 0 ? rows - 1 : -(rows - 1); if (_list_top < 0) _list_top = 0; return; }
      if (y0 >= btn_y) {
        int b = (x0 * 3) / w;
        if (b == 0) { _list_top -= rows - 1; if (_list_top < 0) _list_top = 0; }
        else if (b == 1) { if (_list_top + rows < targetCount()) _list_top += rows - 1; }
        else _task->gotoHomeScreen();
        return;
      }
      if (y0 >= top) {
        int idx = _list_top + (y0 - top) / CHAT_ROW_H;
        if (idx < targetCount()) { _sel = idx; openSelected(); _tmode = T_CHAT; }
      }
      return;
    }
    if (_tmode == T_CHAT) {
      if (swipe) { if (dy > 0) _scroll += 3; else { _scroll -= 3; if (_scroll < 0) _scroll = 0; } return; }
      if (y0 >= btn_y) {
        int b = (x0 * 3) / w;
        if (b == 0) { _in_chat = false; _tmode = T_LIST; rebuildTargets(); }
        else if (b == 1) _tmode = T_QUICK;
        else { _tmode = T_KEYS; _kpage = 0; _shift = false; }
      }
      return;
    }
    if (_tmode == T_QUICK) {
      int n; const char* const* q = quickReplies(n);
      int h = (d.height() - top) / (n + 1);
      if (y0 < top) return;
      int i = (y0 - top) / h;
      if (i < n) sendQuick(q[i]);
      _tmode = T_CHAT; _scroll = 0;
      return;
    }
    // Bildschirmtastatur
    int kh = keyH(d), kt = keysTop(d);
    if (y0 < kt) {
      if (x0 >= w - 18) _tmode = T_CHAT;   // X: zurueck zum Verlauf (Entwurf bleibt)
      return;
    }
    int r = (y0 - kt) / kh;
    if (r < 3) {
      int c = (x0 * 10) / w; if (c > 9) c = 9;
      typeKey(keyLabel(r, c));
      return;
    }
    switch (specialKeyAt(x0, w)) {
      case 0: _shift = !_shift; break;
      case 1: _kpage = _kpage ? 0 : 1; _shift = false; break;
      case 2: typeKey(" "); break;
      case 3: _input_len = utf8Back(_input_len); _input[_input_len] = 0; break;
      default:
        if (_input_len > 0) { sendInput(); if (_input_len == 0) { _tmode = T_CHAT; _scroll = 0; } }
        break;
    }
  }
private:
