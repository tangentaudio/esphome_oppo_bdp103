/*
 * Oppo BDP-103 Blu-Ray Player RS232 Serial Controller
 * Custom Component for ESPHome
 *
 * Steve Richardson (tangentaudio@gmail.com)
 * September, 2022
 *
 */

#pragma once

#include <list>

#include "esphome/core/component.h"
#include "esphome/components/api/custom_api_device.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace oppo_103 {

#define RX_BUFLEN 100
#define TX_BUFLEN RX_BUFLEN

class Oppo103 : public PollingComponent, public uart::UARTDevice, public api::CustomAPIDevice {
public:
  Oppo103();

  void setup() override;
  void update() override;
  void loop() override;

  void set_power_binary_sensor(binary_sensor::BinarySensor *bs) { this->power_binary_sensor_ = bs; }
  void set_playback_status_text_sensor(text_sensor::TextSensor *ts) { this->playback_status_text_sensor_ = ts; }
  void set_disc_type_text_sensor(text_sensor::TextSensor *ts) { this->disc_type_text_sensor_ = ts; }
  void set_audio_type_text_sensor(text_sensor::TextSensor *ts) { this->audio_type_text_sensor_ = ts; }
  void set_input_source_text_sensor(text_sensor::TextSensor *ts) { this->input_source_text_sensor_ = ts; }
  void set_time_code_text_sensor(text_sensor::TextSensor *ts) { this->time_code_text_sensor_ = ts; }

  enum RX_STATES {
    RX_STATE_INIT = 0,
    RX_STATE_HEADER,
    RX_STATE_PAYLOAD,
    RX_STATE_PARSE
  };

  bool power;

protected:
  binary_sensor::BinarySensor *power_binary_sensor_{nullptr};
  text_sensor::TextSensor *playback_status_text_sensor_{nullptr};
  text_sensor::TextSensor *disc_type_text_sensor_{nullptr};
  text_sensor::TextSensor *audio_type_text_sensor_{nullptr};
  text_sensor::TextSensor *input_source_text_sensor_{nullptr};
  text_sensor::TextSensor *time_code_text_sensor_{nullptr};


private:
  void query_initial_state();
  void query_detailed_state();
  void send_pkt(const std::string& cmd);
  void send_next_command();

  void on_power(bool p);
  void on_command(std::string cmd);

  void send_idle_values();
  void parse_packet();

  int rx_state;
  char rx_buf[RX_BUFLEN];
  byte rx_idx;
  bool polling_enabled;
  bool powering_off;

  bool cmd_ready;
  std::list<std::string> cmds;
};

} // namespace
} // namespace
