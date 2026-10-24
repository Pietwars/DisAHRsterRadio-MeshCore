// TX scheduling from meshcore-dev/MeshCore 81dbae592ffad40b4212bc0b37e82b38da08f4bc.
// Only addition: refuse TX after failed/incomplete hardware reconfiguration.
// Included inside namespace mesh for the standalone target only.
#define MIN_TX_BUDGET_AIRTIME_DIV 2
void Dispatcher::updateOriginalMeshCoreTxBudget() {
  unsigned long now = _ms->getMillis();
  unsigned long elapsed = now - last_budget_update;

  float duty_cycle = 1.0f / (1.0f + getAirtimeBudgetFactor());
  unsigned long max_budget = (unsigned long)(getDutyCycleWindowMs() * duty_cycle);
  unsigned long refill = (unsigned long)(elapsed * duty_cycle);

  if (refill > 0) {
    tx_budget_ms += refill;
    if (tx_budget_ms > max_budget) {
      tx_budget_ms = max_budget;
    }
    last_budget_update = now;
  }
}

void Dispatcher::completeOriginalMeshCoreTx() {
      long t = _ms->getMillis() - outbound_start;
      total_air_time += t;
      //Serial.print("  airtime="); Serial.println(t);

      updateOriginalMeshCoreTxBudget();

      if (t > tx_budget_ms) {
        tx_budget_ms = 0;
      } else {
        tx_budget_ms -= t;
      }

      if (tx_budget_ms < MIN_TX_BUDGET_RESERVE_MS) {
        float duty_cycle = 1.0f / (1.0f + getAirtimeBudgetFactor());
        unsigned long needed = MIN_TX_BUDGET_RESERVE_MS - tx_budget_ms;
        next_tx_time = futureMillis((unsigned long)(needed / duty_cycle));
      } else {
        next_tx_time = _ms->getMillis();
      }

      _radio->onSendFinished();
}
void Dispatcher::checkSendOriginalMeshCore() {
  if (!isTxProfileAllowed()) return; // retain applied-hardware failure protection
  if (_mgr->getOutboundCount(_ms->getMillis()) == 0) return;

  updateOriginalMeshCoreTxBudget();

  uint32_t est_airtime = _radio->getEstAirtimeFor(MAX_TRANS_UNIT);
  if (tx_budget_ms < est_airtime / MIN_TX_BUDGET_AIRTIME_DIV) {
    float duty_cycle = 1.0f / (1.0f + getAirtimeBudgetFactor());
    unsigned long needed = est_airtime / MIN_TX_BUDGET_AIRTIME_DIV - tx_budget_ms;
    next_tx_time = futureMillis((unsigned long)(needed / duty_cycle));
    return;
  }

  if (!millisHasNowPassed(next_tx_time)) return;
  if (_radio->isReceiving()) {
    if (cad_busy_start == 0) {
      cad_busy_start = _ms->getMillis();   // record when CAD busy state started
    }

    if (_ms->getMillis() - cad_busy_start > getCADFailMaxDuration()) {
      _err_flags |= ERR_EVENT_CAD_TIMEOUT;

      MESH_DEBUG_PRINTLN("%s Dispatcher::checkSend(): CAD busy max duration reached!", getLogDateTime());
      // channel activity has gone on too long... (Radio might be in a bad state)
      // force the pending transmit below...
    } else {
      next_tx_time = futureMillis(getCADFailRetryDelay());
      return;
    }
  }
  cad_busy_start = 0;  // reset busy state

  outbound = _mgr->getNextOutbound(_ms->getMillis());
  if (outbound) {
    int len = 0;
    uint8_t raw[MAX_TRANS_UNIT];

    raw[len++] = outbound->header;
    if (outbound->hasTransportCodes()) {
      memcpy(&raw[len], &outbound->transport_codes[0], 2); len += 2;
      memcpy(&raw[len], &outbound->transport_codes[1], 2); len += 2;
    }
    raw[len++] = outbound->path_len;
    len += Packet::writePath(&raw[len], outbound->path, outbound->path_len);

    if (len + outbound->payload_len > MAX_TRANS_UNIT) {
      MESH_DEBUG_PRINTLN("%s Dispatcher::checkSend(): FATAL: Invalid packet queued... too long, len=%d", getLogDateTime(), len + outbound->payload_len);
      _mgr->free(outbound);
      outbound = NULL;
    } else {
      memcpy(&raw[len], outbound->payload, outbound->payload_len); len += outbound->payload_len;

      uint32_t max_airtime = _radio->getEstAirtimeFor(len)*3/2;
      outbound_start = _ms->getMillis();
      bool success = _radio->startSendRaw(raw, len);
      if (!success) {
        MESH_DEBUG_PRINTLN("%s Dispatcher::loop(): ERROR: send start failed!", getLogDateTime());

        logTxFail(outbound, outbound->getRawLength());

        releasePacket(outbound);  // return to pool
        outbound = NULL;
        return;
      }
      outbound_expiry = futureMillis(max_airtime);

    #if MESH_PACKET_LOGGING
      Serial.print(getLogDateTime());
      Serial.printf(": TX, len=%d (type=%d, route=%s, payload_len=%d)",
            len, outbound->getPayloadType(), outbound->isRouteDirect() ? "D" : "F", outbound->payload_len);
      if (outbound->getPayloadType() == PAYLOAD_TYPE_PATH || outbound->getPayloadType() == PAYLOAD_TYPE_REQ
        || outbound->getPayloadType() == PAYLOAD_TYPE_RESPONSE || outbound->getPayloadType() == PAYLOAD_TYPE_TXT_MSG) {
        Serial.printf(" [%02X -> %02X]\n", (uint32_t)outbound->payload[1], (uint32_t)outbound->payload[0]);
      } else {
        Serial.printf("\n");
      }
    #endif
    }
  }
}

