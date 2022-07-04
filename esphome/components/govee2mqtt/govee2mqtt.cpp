#include "govee2mqtt.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace govee2mqtt{

#define	MQTT_TOPIC_PREFIX "govee2mqtt/devices/%04x/"
#define MQTT_PAIRING_TOPIC "govee2mqtt/pairing/newdevice/state"


#define HA_MQTT_TOPIC_PRE(type) "homeassistant/" type "/"
#define HA_MQTT_TOPIC_POST(dev) "/" dev "/config"

static const char *const TAG = "govee2mqtt";

void Govee2MqttComponent::setup() {
	ESP_LOGD(TAG, "Govee MQTT Setup...");
}

void Govee2MqttComponent::set_dev_list ( DeviceType *dev_list ) {
	loc_dev_list_.clear();
	auto const & y = dev_list->value();
	for ( auto it = y.begin(); it!= y.end(); ++it ) {
		loc_dev_list_[it->first] = it->second;
	}
}

void Govee2MqttComponent::set_pairing_mode(bool en ) {
	pairing_mode_ = en;
	ESP_LOGD(TAG, "%s pairing mode...", en ? "Enter" : "Leave" );
	// clear pairing topic either enter or leave pairing mode
	if ( mqtt_id_) mqtt_id_->publish(MQTT_PAIRING_TOPIC,"{}");
}

void Govee2MqttComponent::dump_config() {
	ESP_LOGCONFIG(TAG, "Govee MQTT Devices:");
	for ( auto it = loc_dev_list_.begin(); it!= loc_dev_list_.end(); ++it ) {
		ESP_LOGCONFIG(TAG,"  %04x - %s",it->first, it->second[0].c_str());
	}
}

void Govee2MqttComponent::play_govee_data(GoveeDataType x )  {
	auto const & y = loc_dev_list_.find(x.address);
	if ( y != loc_dev_list_.end()) {
		uint32_t	millis_ = millis();
		char topic[128], payload[128];
		auto const & model = y->second[0];

		if ( model == "H5054" ) {
			   switch (x.command ) {
			   case (0xfa):
				send_mqtt_bool_msg_ (x.address, 1, "button");
				delay_button_[x.address] = millis_;
				break;
			   case (0xfb):
				{
					auto const &z = delay_waterleak_.find(x.address);
					if ( z == delay_waterleak_.end() ) {
						send_mqtt_bool_msg_ (x.address, 1, "waterleak");
					}
				}
				delay_waterleak_[x.address] = millis_;
				break;
			   case (0xfc):
				snprintf(topic,128, MQTT_TOPIC_PREFIX"battery_level/state", x.address);
				snprintf(payload,128, "%d", (x.cmdopt>>8) & 0xff);
				ESP_LOGD("Covee message", "Device %04x: Battery Level = %d%%", x.address, (x.cmdopt>>8));
				if ( mqtt_id_) mqtt_id_->publish(topic,payload);
				break;
			   default:
				send_mqtt_raw_msg_(x, "unknown_cmd");
				break;
			}
		}else {
			send_mqtt_raw_msg_(x, "unknown_dev_raw");
		}
	}else{
		if ( pairing_mode_ ) {
			char payload[128];
			snprintf(payload,128,"{\"Device\":\"%04x\", \"Command\":\"%02x\", \"Options\":\"%04x\"}", x.address, x.command, x.cmdopt);
			if ( mqtt_id_) mqtt_id_->publish(MQTT_PAIRING_TOPIC,payload);
			ESP_LOGD ("Govee Pairing Message", "%s",payload);
		}else {
			ESP_LOGD ("Govee Unknown Device", "%04x is not in device list, ignored.", x.address);
		}
	}
}

void Govee2MqttComponent::send_mqtt_raw_msg_ (const GoveeDataType & x, const char *sub){
	char topic[128], payload[128];
	snprintf(topic,128, MQTT_TOPIC_PREFIX "%s/state", x.address, sub);
	snprintf(payload,128, "{\"Command\":\"%02X\", \"Options\":\"%04X\"}", x.command, x.cmdopt);
	ESP_LOGD("Covee Unknown", "Type:%s, %s", sub, payload );
	if ( mqtt_id_) mqtt_id_->publish(topic,payload);
}


void Govee2MqttComponent::send_mqtt_bool_msg_ (govee_addr_sz address, bool on, const char *type){
	char topic[128];
	const char * state_msg = on ? "on" : "off" ;

	ESP_LOGD("Covee message", "Device %04x: %s %s", address, type, state_msg );
	snprintf(topic, 128, MQTT_TOPIC_PREFIX "%s/state", address, type );
	if ( mqtt_id_ ) mqtt_id_->publish(topic, state_msg);
}

void Govee2MqttComponent::loop() {
	uint32_t	millis_  = millis();
	if ( millis_ - last_check_millis_ < 500 ) {
		// be nice only check every 500ms
		return;
	}
	last_check_millis_ = millis_;
	for (std::map<govee_addr_sz,uint32_t>::iterator it = delay_button_.begin(); it!= delay_button_.end(); ++it ) {
		if ( (millis_ - it->second) > 1000 ) {
			// button released for more than a second, send off message
			send_mqtt_bool_msg_ (it->first, 0, "button");
			delay_button_.erase(it);
		}
	}
	for (std::map<govee_addr_sz,uint32_t>::iterator it = delay_waterleak_.begin(); it!= delay_waterleak_.end(); ++it ) {
		if ( (millis_ - it->second) > 7000 ) {
			// waterleak released for more than 7 seconds, send off message
			send_mqtt_bool_msg_ (it->first, 0, "waterleak");
			delay_waterleak_.erase(it);
		}
	}

}

void Govee2MqttComponent::mqtt_set_dev_list(JsonObjectConst root ){
	ESP_LOGD("Govee Mqtt", "Set devices");
	loc_dev_list_.clear();
	mqtt_add_remove_dev_list_raw_(root, false);
}

void Govee2MqttComponent::mqtt_add_dev_list(JsonObjectConst root ){
	ESP_LOGD("Govee Mqtt", "Add devices");
	mqtt_add_remove_dev_list_raw_(root, false);
}

void Govee2MqttComponent::mqtt_remove_dev_list(JsonObjectConst root ){
	ESP_LOGD("Govee Mqtt", "Delete devices");
	mqtt_add_remove_dev_list_raw_(root, true);
}

void Govee2MqttComponent::mqtt_add_remove_dev_list_raw_(JsonObjectConst root, bool remove_mode ){
	for ( auto it = root.begin(); it != root.end(); ++it ) {
		auto info = it->value().as<JsonObjectConst>();
		const char * addr_str = it->key().c_str();
		const std::string & model = info["model"].as<std::string>();
		const std::string & name = info["name"].as<std::string>();
		char * ret_ptr;
		uint32_t addr = strtoul(addr_str,&ret_ptr, 16);
		if ( ret_ptr != nullptr && ret_ptr[0] != '\0' ) {
			ESP_LOGW("Govee Mqtt", "  Skip Device \"%s\", address is invalid", addr_str);
			continue;
		}
		if ( addr > 0xffff ) {
			ESP_LOGW("Govee Mqtt", "  Skip Device \"%s\", address is to high", addr_str);
			continue;
		}
		auto const & find_dev = loc_dev_list_.find(addr);
		std::vector<std::string> dev_info;
		dev_info.push_back(std::move(model));
		dev_info.push_back(std::move(name));
		if ( find_dev != loc_dev_list_.end()) {
			if ( remove_mode ) {
				loc_dev_list_.erase(find_dev);
				ESP_LOGD("Govee Mqtt", "  Removed Device %04x",addr);
			}else {
				ESP_LOGW("Govee Mqtt", "  Skip Device %04x: try to add an existing device",addr);
			}
		}else {
			if ( remove_mode ) {
				ESP_LOGW("Govee Mqtt", "  Skip Device %04x: try to remove an non-existing device",addr);
			}else {
				ESP_LOGD("Govee Mqtt", "  Added Device %04x",addr);
				loc_dev_list_[addr] = dev_info;
				send_discovery_unit_(addr, dev_info);
			}
		}

//		ESP_LOGD("Govee Mqtt", "Device=%s, Model=\"%s\", Name=\"%s\"", it->key().c_str(), .c_str(),info["name"].as<std::string>().c_str() );
	}

}

void Govee2MqttComponent::send_discovery(){
	ESP_LOGD("Govee2MQTT", "Send discovery to HA");
	for ( auto it = loc_dev_list_.begin(); it!= loc_dev_list_.end(); ++it ) {
		send_discovery_unit_(it->first, it->second);
	}
}

void Govee2MqttComponent::send_discovery_unit_(govee_addr_sz address, const std::vector<std::string> &unit ){
	char addr[20];
	char addr_short[6];
	std::string topic;
	std::string payload;
	snprintf(addr_short,6,"%04x",address);
	std::strcpy(addr, "govee_0x");
	std::strcat(addr,addr_short);
	auto const & model = unit[0];
	if ( model == "H5054" ) {
		// provide leakage
		topic = (std::string)HA_MQTT_TOPIC_PRE("binary_sensor") + addr + HA_MQTT_TOPIC_POST("water_leak");
		payload = (std::string) "{\"device\":{\"identifiers\":[\"" + addr + "\"],\"manufacturer\":\"Govee\",\"model\":\"" + model + "\",\"name\":\"" + unit[1]+"\"},"
			"\"device_class\":\"moisture\","
			"\"name\": \"" + unit[1] + " water leak\","
			"\"payload_off\" : \"off\","
			"\"payload_on\" : \"on\","
			"\"state_topic\": \"govee2mqtt/devices/" + addr_short + "/waterleak/state\","
			"\"unique_id\": \"" + addr + "_waterleak\""
			"}";
		mqtt_id_->publish(topic, payload, 0, true);
		//ESP_LOGD("Govee Discovery", "  %s:%s",topic.c_str(),payload.c_str());

		// provide battery
		topic = (std::string)HA_MQTT_TOPIC_PRE("sensor") + addr + HA_MQTT_TOPIC_POST("battery");
		payload = (std::string) "{\"device\":{\"identifiers\":[\"" + addr + "\"],\"manufacturer\":\"Govee\",\"model\":\"" + model + "\",\"name\":\"" + unit[1]+"\"},"
			"\"device_class\":\"battery\","
			"\"enabled_by_default\": true,"
			"\"entity_category\":\"diagnostic\","
			"\"name\": \"" + unit[1] + " battery\","
			"\"state_class\": \"measurement\","
			"\"state_topic\": \"govee2mqtt/devices/" + addr_short + "/battery_level/state\","
			"\"unique_id\": \"" + addr + "_battery\","
			"\"unit_of_measurement\": \"%\""
			"}";
		mqtt_id_->publish(topic, payload, 0, true);
		//ESP_LOGD("Govee Discovery", "  %s:%s",topic.c_str(),payload.c_str());

		// provide button
		topic = (std::string)HA_MQTT_TOPIC_PRE("binary_sensor") + addr + HA_MQTT_TOPIC_POST("button");
		payload = (std::string) "{\"device\":{\"identifiers\":[\"" + addr + "\"],\"manufacturer\":\"Govee\",\"model\":\"" + model + "\",\"name\":\"" + unit[1]+"\"},"
			"\"name\": \"" + unit[1] + " button\","
			"\"payload_off\" : \"off\","
			"\"payload_on\" : \"on\","
			"\"state_topic\": \"govee2mqtt/devices/" + addr_short + "/button/state\","
			"\"unique_id\": \"" + addr + "_button\""
			"}";
		mqtt_id_->publish(topic, payload, 0, true);
		//ESP_LOGD("Govee Discovery", "  %s:%s",topic.c_str(),payload.c_str());

	}

}

}	// namespace remote_receiver
}	// namespace esphome
