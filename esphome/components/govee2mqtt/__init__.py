import esphome.codegen as cg
from esphome import config_validation as cv
from esphome import automation
from esphome.automation import maybe_simple_id

from esphome.const import (
    CONF_ID,
)

CODEOWNERS = ["@hlyi"]

CONF_DEVLIST = "dev_list"
# CONF_RECEIVERID = "receiver_id"
CONF_MQTTID = "mqtt_id"

globals_ns = cg.esphome_ns.namespace("globals")
GlobalsComponent = globals_ns.class_("GlobalsComponent", cg.Component)

remote_receiver_nf_ns = cg.esphome_ns.namespace("remote_receiver_nf")
RemoteReceiverNFComponent = remote_receiver_nf_ns.class_(
    "RemoteReceiverNFComponent", cg.Component
)
mqtt_ns = cg.esphome_ns.namespace("mqtt")
MQTTClientComponent = mqtt_ns.class_("MQTTClientComponent", cg.Component)

RestoringGlobalsComponent = globals_ns.class_("RestoringGlobalsComponent", cg.Component)

GlobalVarSetAction = globals_ns.class_("GlobalVarSetAction", automation.Action)

govee2mqtt_ns = cg.esphome_ns.namespace("govee2mqtt")
Govee2MqttComponent = govee2mqtt_ns.class_("Govee2MqttComponent", cg.Component)

PairingOffAction = govee2mqtt_ns.class_("PairingOffAction", automation.Action)
PairingOnAction = govee2mqtt_ns.class_("PairingOnAction", automation.Action)
SendDiscoveryAction = govee2mqtt_ns.class_("SendDiscoveryAction", automation.Action)
PlayGoveeDataAction = govee2mqtt_ns.class_("PlayGoveeDataAction", automation.Action)
MqttSetDevListAction = govee2mqtt_ns.class_("MqttSetDevListAction", automation.Action)
MqttAddDevListAction = govee2mqtt_ns.class_("MqttAddDevListAction", automation.Action)
MqttRemoveDevListAction = govee2mqtt_ns.class_(
    "MqttRemoveDevListAction", automation.Action
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Govee2MqttComponent),
        cv.Required(CONF_DEVLIST): cv.use_id(GlobalsComponent),
        #        cv.Required(CONF_RECEIVERID): cv.use_id(RemoteReceiverNFComponent),
        cv.Required(CONF_MQTTID): cv.use_id(MQTTClientComponent),
    }
).extend(cv.COMPONENT_SCHEMA)

GOVEE2MQTT_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(Govee2MqttComponent),
    }
)


@automation.register_action(
    "govee2mqtt.mqtt_add_dev_list", MqttAddDevListAction, GOVEE2MQTT_ACTION_SCHEMA
)
async def govee2mqtt_mqtt_add_dev_list_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


@automation.register_action(
    "govee2mqtt.mqtt_remove_dev_list", MqttRemoveDevListAction, GOVEE2MQTT_ACTION_SCHEMA
)
async def govee2mqtt_mqtt_remove_dev_list_to_code(
    config, action_id, template_arg, args
):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


@automation.register_action(
    "govee2mqtt.mqtt_set_dev_list", MqttSetDevListAction, GOVEE2MQTT_ACTION_SCHEMA
)
async def govee2mqtt_mqtt_set_dev_list_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


@automation.register_action(
    "govee2mqtt.play_govee_data", PlayGoveeDataAction, GOVEE2MQTT_ACTION_SCHEMA
)
async def govee2mqtt_play_govee_data_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


@automation.register_action(
    "govee2mqtt.send_discovery", SendDiscoveryAction, GOVEE2MQTT_ACTION_SCHEMA
)
async def govee2mqtt_send_discovery_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


@automation.register_action(
    "govee2mqtt.pairing_on", PairingOnAction, GOVEE2MQTT_ACTION_SCHEMA
)
async def govee2mqtt_pairing_on_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


@automation.register_action(
    "govee2mqtt.pairing_off", PairingOffAction, GOVEE2MQTT_ACTION_SCHEMA
)
async def govee2mqtt_pairing_off_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)


async def to_code(config):

    var = cg.new_Pvariable(config[CONF_ID])
    mqtt_ = await cg.get_variable(config[CONF_MQTTID])
    cg.add(var.set_mqtt_id(mqtt_))
    #    receiver_ = await cg.get_variable(config[CONF_RECEIVERID])
    #    cg.add(var.set_receiver_id(receiver_))
    devlist_ = await cg.get_variable(config[CONF_DEVLIST])
    cg.add(var.set_dev_list(devlist_))

    await cg.register_component(var, config)
