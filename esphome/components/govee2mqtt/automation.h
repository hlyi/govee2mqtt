#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/govee2mqtt/govee2mqtt.h"

namespace esphome {
namespace govee2mqtt {


template<typename... Ts> class PairingOffAction : public Action<Ts...> {
 public:
  PairingOffAction ( Govee2MqttComponent *govee2mqtt) : govee2mqtt_(govee2mqtt) {}

  void play(Ts... x) override { this->govee2mqtt_->set_pairing_mode(false); }

 protected:
  Govee2MqttComponent *govee2mqtt_;
};

template<typename... Ts> class PairingOnAction : public Action<Ts...> {
 public:
  PairingOnAction ( Govee2MqttComponent *govee2mqtt) : govee2mqtt_(govee2mqtt) {}

  void play(Ts... x) override { this->govee2mqtt_->set_pairing_mode(true); }

 protected:
  Govee2MqttComponent *govee2mqtt_;
};

template<typename... Ts> class SendDiscoveryAction : public Action<Ts...> {
 public:
  SendDiscoveryAction ( Govee2MqttComponent *govee2mqtt) : govee2mqtt_(govee2mqtt) {}

  void play(Ts... x) override { this->govee2mqtt_->send_discovery(); }

 protected:
  Govee2MqttComponent *govee2mqtt_;
};

template<typename... Ts> class PlayGoveeDataAction : public Action<Ts...> {
 public:
  PlayGoveeDataAction ( Govee2MqttComponent *govee2mqtt) : govee2mqtt_(govee2mqtt) {}

  void play(Ts... x) override { this->govee2mqtt_->play_govee_data(x...); }

 protected:
  Govee2MqttComponent *govee2mqtt_;
};

template<typename... Ts> class MqttSetDevListAction : public Action<Ts...> {
 public:
  MqttSetDevListAction ( Govee2MqttComponent *govee2mqtt) : govee2mqtt_(govee2mqtt) {}

  void play(Ts... x) override { this->govee2mqtt_->mqtt_set_dev_list(x...); }

 protected:
  Govee2MqttComponent *govee2mqtt_;
};


template<typename... Ts> class MqttAddDevListAction : public Action<Ts...> {
 public:
  MqttAddDevListAction ( Govee2MqttComponent *govee2mqtt) : govee2mqtt_(govee2mqtt) {}

  void play(Ts... x) override { this->govee2mqtt_->mqtt_add_dev_list(x...); }

 protected:
  Govee2MqttComponent *govee2mqtt_;
};


template<typename... Ts> class MqttRemoveDevListAction : public Action<Ts...> {
 public:
  MqttRemoveDevListAction ( Govee2MqttComponent *govee2mqtt) : govee2mqtt_(govee2mqtt) {}

  void play(Ts... x) override { this->govee2mqtt_->mqtt_remove_dev_list(x...); }

 protected:
  Govee2MqttComponent *govee2mqtt_;
};



}  // namespace govee2mqtt
}  // namespace esphome
