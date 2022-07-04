#pragma once

#include "esphome/components/mqtt/mqtt_client.h"
#include "esphome/components/globals/globals_component.h"
#include "esphome/components/remote_base/govee_protocol.h"
#include <stdio.h>

using MqttClientType = esphome::mqtt::MQTTClientComponent ;
//using ReceiverType = esphome::remote_receiver_nf::RemoteReceiverNFComponent ;
using DeviceType = esphome::globals::GlobalsComponent<std::map<int, std::vector<std::string>>> ;
using GoveeDataType = esphome::remote_base::GoveeData ;
using GoveeTriggerType = esphome::remote_base::GoveeTrigger ;
using GoveeActionType = esphome::Action<GoveeDataType> ;
using GoveeAutomationType = esphome::Automation<GoveeDataType>;

namespace esphome {
namespace govee2mqtt{

using govee_addr_sz	= uint16_t;


class Govee2MqttComponent : public Component
{
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  void set_mqtt_id          ( MqttClientType * mqtt_id    ) { this->mqtt_id_ = mqtt_id; };
  void set_dev_list         ( DeviceType     * dev_list   ) ;
  void send_discovery       ( );
  void set_pairing_mode     ( bool en);
  void play_govee_data      ( GoveeDataType x );
  bool pairing_mode         ( ) { return pairing_mode_; };
  void mqtt_set_dev_list    ( const JsonObjectConst json );
  void mqtt_add_dev_list    ( const JsonObjectConst json );
  void mqtt_remove_dev_list ( const JsonObjectConst json );

 protected:
  void send_discovery_unit_ ( govee_addr_sz address, const std::vector<std::string> &unit );
  void mqtt_add_remove_dev_list_raw_    ( const JsonObjectConst json, bool remove_mode = false );
  void send_mqtt_bool_msg_  ( govee_addr_sz address, bool on, const char * type );
  void send_mqtt_raw_msg_   ( const GoveeDataType &x, const char * sub );

  MqttClientType                     * mqtt_id_{nullptr};
//  DeviceType                         * dev_list_{nullptr};
  std::map<int, std::vector<std::string>> loc_dev_list_{};
  std::map<govee_addr_sz,uint32_t>   delay_button_{};
  std::map<govee_addr_sz,uint32_t>   delay_waterleak_{};
  uint32_t                           last_check_millis_{0};
  bool                               pairing_mode_ {false};
};


}  // namespace remote_receiver
}  // namespace esphome
