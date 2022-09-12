/*
 * Oppo BDP-103 Blu-Ray Player RS232 Serial Controller
 * Custom Component for ESPHome
 *
 * Steve Richardson (tangentaudio@gmail.com)
 * September, 2022
 *
 * See http://download.oppodigital.com/BDP103/BDP-103_RS232_Protocol_v1.1.pdf
 */

#include "esphome.h"
#include "esphome/core/log.h"
#include "oppo_bdp103.h"

namespace esphome {
namespace oppo_103 {

static const char* TAG = "oppo_103.component";

Oppo103::Oppo103() :
  PollingComponent(500), rx_state(0), rx_idx(0), polling_enabled(true), cmd_ready(true)
{
}

void Oppo103::setup() {
  register_service(&Oppo103::on_power, "oppo_power", {"power"});
  register_service(&Oppo103::on_command, "oppo_command", {"cmd"});
}

void Oppo103::update() {
  static bool last_connected = false;
  bool connected = is_connected();

  if (last_connected != connected) {
    query_initial_state();
  }
  last_connected = connected;

  if (polling_enabled) {
    send_next_command();
  }
}


void Oppo103::on_power(bool p) {
  ESP_LOGD(TAG, "power service: %s", p ? "ON" : "OFF");

  if (p) {
    cmds.push_back("PON");
  } else {
    cmds.push_back("POF");
  }
  send_next_command();
}

void Oppo103::query_initial_state() {
  cmds.clear();

  cmds.push_back("SVM 3");
  cmds.push_back("QPW");

}

void Oppo103::query_detailed_state() {
  cmds.push_back("QPL");
  cmds.push_back("QDT");
  cmds.push_back("QAT");
  cmds.push_back("QIS");
}


void Oppo103::send_pkt(const std::string& cmd) {
  char pkt[TX_BUFLEN];

  snprintf(pkt, TX_BUFLEN, "#%s\r", cmd.c_str());

  for (int i=0; pkt[i]!=0; i++) {
    write(pkt[i]);
  }

  ESP_LOGD(TAG, "sent serial pkt: %s", pkt);
}

void Oppo103::send_next_command() {
  if (cmd_ready && cmds.size()) {
    const std::string c = cmds.back();
    cmd_ready = false;
    send_pkt(c);
    ESP_LOGD(TAG, "next command is %s from list", c.c_str());
    cmds.pop_back();
  }
}

void Oppo103::on_command(std::string cmd) {
  send_pkt(cmd.c_str());
}

void Oppo103::send_idle_values() {

  power = false;

  if (this->power_binary_sensor_ != nullptr)
    this->power_binary_sensor_->publish_state(power);
}

void Oppo103::parse_packet() {
  std::string pkt = std::string(rx_buf);

  cmd_ready = true;

  if (pkt.length() < 3) {
    ESP_LOGE(TAG, "Can't parse short packet.");
    return;
  }

  if (pkt.substr(3, 1) == " ") {
    // verbose response
    std::string rstatus = pkt.substr(0, 3);
    std::string rparam = pkt.substr(4);

    if (rparam.substr(0, 3) == "OK " && rstatus.substr(0, 1) == "Q")
      rparam = rparam.substr(3);

    if (rstatus == "SVM") {
      if (rparam == "OK 3") {
        ESP_LOGI(TAG, "set verbose mode success.");
      }
    } else if (rstatus == "UPL" || rstatus == "QPL") {
      ESP_LOGI(TAG, "Playback Status now %s", rparam.c_str());
      if (this->playback_status_text_sensor_ != nullptr)
        this->playback_status_text_sensor_->publish_state(rparam);
    } else if (rstatus == "UDT" || rstatus == "QDT") {
      ESP_LOGI(TAG, "Disc Type update %s", rparam.c_str());
      if (this->disc_type_text_sensor_ != nullptr)
        this->disc_type_text_sensor_->publish_state(rparam);
    } else if (rstatus == "UAT" || rstatus == "QAT") {
      ESP_LOGI(TAG, "Audio Type update %s", rparam.c_str());
      if (this->audio_type_text_sensor_ != nullptr)
        this->audio_type_text_sensor_->publish_state(rparam);
    } else if (rstatus == "UIS" || rstatus == "QIS") {
      ESP_LOGI(TAG, "Input Source update %s", rparam.c_str());
      if (this->input_source_text_sensor_ != nullptr)
        this->input_source_text_sensor_->publish_state(rparam);
    } else if (rstatus == "UTC") {
      ESP_LOGI(TAG, "Time Code %s", rparam.c_str());
      if (this->time_code_text_sensor_ != nullptr)
        this->time_code_text_sensor_->publish_state(rparam);
    } else if (rstatus == "UVO") {
        ESP_LOGI(TAG, "Video modes %s", rparam.c_str());
    } else if (rstatus == "U3D") {
        ESP_LOGI(TAG, "3D modes %s", rparam.c_str());
    } else if (rstatus == "UPW" || rstatus == "QPW") {
      if (rparam == "1" || rparam == "ON") {
        ESP_LOGI(TAG, "Player power turned on.");
        power = true;
        if (this->power_binary_sensor_ != nullptr)
          this->power_binary_sensor_->publish_state(power);

        query_detailed_state();
      } else if (rparam == "0" || rparam == "OFF") {
        ESP_LOGI(TAG, "Player power turned off.");
        power = false;
        if (this->power_binary_sensor_ != nullptr)
          this->power_binary_sensor_->publish_state(power);
      } else {
        ESP_LOGI(TAG, "Unparsed command: %s [%s]", rstatus.c_str(), rparam.c_str());
      }
    }
  } else {
    // regular response
    std::string resp = std::string(rx_buf);
    ESP_LOGD(TAG, "standard response received");

    if (resp == "OK 2") {
      ESP_LOGI(TAG, "set verbose mode success.");
    }
  }

}

void Oppo103::loop() {
  static uint32_t timeout;

  switch (rx_state) {
    case RX_STATE_INIT:
      flush();
      polling_enabled = true;
      send_idle_values();

      rx_state = RX_STATE_HEADER;
      break;

    case RX_STATE_HEADER:
      if (available()) {
        if (read() == '@') {
          rx_idx = 0;
          rx_state = RX_STATE_PAYLOAD;
          timeout = millis();
          ESP_LOGD(TAG, "Got @ header");
        }
      }
      break;

    case RX_STATE_PAYLOAD:
      while (available()) {
        byte b;
        // this isn't documented in the BDP-103 spec, but sometimes the CR's seem to be missing
        // and multiple packets get squished together.  this handles that case
        if (peek_byte(&b) && b != '@')
           b = read();

        if ((b == 0x0D || b == '@') && rx_idx < RX_BUFLEN) {
          // end of line
          rx_buf[rx_idx] = 0;
          rx_state = RX_STATE_PARSE;
          ESP_LOGD(TAG, "Got packet, parsing: %s", rx_buf);
        } else if (rx_idx < RX_BUFLEN) {
          // payload byte
          rx_buf[rx_idx++] = b;
        } else {
          ESP_LOGE(TAG, "RX Buf overflow!");
          rx_state = RX_STATE_INIT;
        }

        if (millis() - timeout > 100) {
          ESP_LOGE(TAG, "Timed out waiting for payload.");
          rx_state = RX_STATE_INIT;
          break;
        }

        if (b == '@') {
          ESP_LOGD(TAG, "Got multi pkt");
          break;
        }
      }
      break;

    case RX_STATE_PARSE:
      parse_packet();
      rx_state = RX_STATE_HEADER;
      break;

    default:
      rx_state = RX_STATE_INIT;
      break;
  }
}

} // namespace
} // namespace
