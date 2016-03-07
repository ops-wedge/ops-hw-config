/*
 * (c) Copyright 2015 Hewlett Packard Enterprise Development LP
 *
 *    Licensed under the Apache License, Version 2.0 (the "License"); you may
 *    not use this file except in compliance with the License. You may obtain
 *    a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *    License for the specific language governing permissions and limitations
 *    under the License.
 */
#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <stdlib.h>
#include <string.h>

using namespace std;

#include "yaml-cpp/yaml.h"
#include "config-yaml.h"


typedef struct {
    map<string, YamlDevice> device_map;

    map<string, YamlBus>    bus_map;

    map<string, YamlFile>   file_map;

    YamlSubsysInfo          subsys_info;

    vector<YamlSensor>      sensors;
    YamlThermalInfo         thermal;

    YamlPortInfo            port_info;
    vector<YamlPort>        ports;

    vector<YamlFanFru>      fan_frus;
    YamlFanInfo             fan_info;

    YamlPsuInfo             psu_info;
    vector<YamlPsu>         psus;

    YamlLedInfo             led_info;
    vector<YamlLedType>     led_types;
    vector<YamlLed>         leds;

    YamlFruInfo             fru_info;

    vector<i2c_op>          init_ops;

    string                  dir_name;
} YamlSubsystem;

typedef struct {
    map<string, YamlSubsystem*> subsystem_map;
} YamlConfigHandlePrivate;

static void operator >> (const YAML::Node &node, YamlSubsysInfo &sub_info)
{
    string str;

    node["subsystem_info"] >> str;
    sub_info.info = strdup(str.c_str());
}

static void operator >> (const YAML::Node &node, YamlPortInfo &port_info)
{
    node["number_ports"] >> port_info.number_ports;
    node["max_port_speed"] >> port_info.max_port_speed;
    node["max_transmission_unit"] >> port_info.max_transmission_unit;
    node["max_lag_count"] >> port_info.max_lag_count;
    node["max_lag_member_count"] >> port_info.max_lag_member_count;
    node["L3_port_requires_internal_VLAN"] >> port_info.l3_port_requires_internal_vlan;
}

static void operator >> (const YAML::Node &node, vector<string> &list)
{
    for (size_t i = 0; i < node.size(); i++) {
        string item;
        node[i] >> item;
        list.push_back(item);
    }
}

static void operator >> (const YAML::Node &node, vector<unsigned char> &bytes)
{
    for (size_t i = 0; i < node.size(); i++) {
        string str;
        unsigned char item;
        node[i] >> str;
        item = (unsigned char)strtoul(str.c_str(), 0, 0);
        bytes.push_back(item);
    }
}

static void operator >> (const YAML::Node &node, i2c_op &op)
{
    string str;

    op.direction = WRITE;
    node["device"] >> str;
    op.device = strdup(str.c_str());
    node["register"] >> str;

    if (str == "NONE") {
        op.set_register = false;
        op.register_address = 0;
    } else {
        op.set_register = true;
        op.register_address = (unsigned char)strtoul(str.c_str(), 0, 0);
    }

    vector<unsigned char> bytes;

    node["data"] >> bytes;

    op.byte_count = bytes.size();

    op.data = (unsigned char *)malloc(op.byte_count);

    for (size_t idx = 0; idx < bytes.size(); idx++) {
        op.data[idx] = bytes[idx];
    }

    op.negative_polarity = false;

    if (const YAML::Node *pNode = node.FindValue("polarity")) {
        string str;
        *pNode >> str;

        if (str == "negative") {
            op.negative_polarity = true;
        }
    }
}

static void operator >> (const YAML::Node &node, vector<i2c_op> &ops)
{
    for (size_t i = 0; i < node.size(); i++) {
        i2c_op item;
        node[i] >> item;
        ops.push_back(item);
    }
}

static void operator >> (const YAML::Node &node, YamlDevice &device)
{
    string str;

    node["name"] >> str;
    device.name = strdup(str.c_str());

    node["bus"] >> str;
    device.bus = strdup(str.c_str());

    node["dev_type"] >> str;
    device.dev_type = strdup(str.c_str());

    node["address"] >> device.address;

    if (const YAML::Node *pName = node.FindValue("pre")) {
        vector<i2c_op> pre;
        *pName >> pre;

        device.pre = (i2c_op **)calloc(sizeof(i2c_op *), pre.size() + 1);

        for (size_t idx = 0; idx < pre.size(); idx++) {
            device.pre[idx] = (i2c_op *)malloc(sizeof(i2c_op));
            *device.pre[idx] = pre[idx];
        }
    } else {
        device.pre = NULL;
    }

    if (const YAML::Node *pName = node.FindValue("post")) {
        vector<i2c_op> post;
        *pName >> post;

        device.post = (i2c_op **)calloc(sizeof(i2c_op), post.size() + 1);

        for (size_t idx = 0; idx < post.size(); idx++) {
            device.post[idx] = (i2c_op *)malloc(sizeof(i2c_op));
            *device.post[idx] = post[idx];
        }
    } else {
        device.post = NULL;
    }
}

static void operator >> (const YAML::Node &node, map<string, YamlDevice> &devices)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlDevice device;
        node[i] >> device;
        devices[device.name] = device;
    }
}

static void operator >> (const YAML::Node &node, YamlAlarmThresholds &thresh)
{
    node["emergency_on"] >> thresh.emergency_on;
    node["emergency_off"] >> thresh.emergency_off;
    node["critical_on"] >> thresh.critical_on;
    node["critical_off"] >> thresh.critical_off;
    node["max_on"] >> thresh.max_on;
    node["max_off"] >> thresh.max_off;
    node["min"] >> thresh.min;
    node["low_crit"] >> thresh.low_crit;
}

static void operator >> (const YAML::Node &node, YamlFanThresholds &thresh)
{
    node["max_on"] >> thresh.max_on;
    node["max_off"] >> thresh.max_off;
    node["fast_on"] >> thresh.fast_on;
    node["fast_off"] >> thresh.fast_off;
    node["medium_on"] >> thresh.medium_on;
    node["medium_off"] >> thresh.medium_off;
}

static void operator >> (const YAML::Node &node, YamlSensor &sensor)
{
    string str;
    node["number"] >> sensor.number;

    node["location"] >> str;
    sensor.location = strdup(str.c_str());

    node["device"] >> str;
    sensor.device = strdup(str.c_str());

    node["sensor_type"] >> str;
    sensor.type = strdup(str.c_str());

    node["alarm_thresholds"] >> sensor.alarm_thresholds;
    node["fan_thresholds"] >> sensor.fan_thresholds;
}

static void operator >> (const YAML::Node &node, vector<YamlSensor> &sensors)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlSensor sensor;
        node[i] >> sensor;
        sensors.push_back(sensor);
    }
}

static void operator >> (const YAML::Node &node, i2c_bit_op &op)
{
    string str;

    node["device"] >> str;
    op.device = strdup(str.c_str());

    node["register"] >> str;
    op.register_address = (unsigned char)strtoul(str.c_str(), 0, 0);

    node["bitmask"] >> str;
    op.bit_mask = (unsigned char)strtoul(str.c_str(), 0, 0);
    op.register_size = str.size()/2 - 1;    // must be hex bytes with leading 0x!

    if (const YAML::Node *pNode = node.FindValue("polarity")) {
        string str;
        *pNode >> str;

        if (str == "negative") {
            op.negative_polarity = true;
        }
    }
}

static void operator >> (const YAML::Node &node, YamlQsfp28ModuleSignals &signals)
{
    if (const YAML::Node *pNode = node.FindValue("qsfp28p_reset")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfp28p_reset = op;
    }
    if (const YAML::Node *pNode = node.FindValue("qsfp28p_mod_present")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfp28p_mod_present = op;
    }
    if (const YAML::Node *pNode = node.FindValue("qsfp28p_interrupt")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfp28p_interrupt = op;
    }
    if (const YAML::Node *pNode = node.FindValue("qsfp28p_interrupt_mask")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfp28p_interrupt_mask = op;
    }
}

static void operator >> (const YAML::Node &node, YamlQsfpModuleSignals &signals)
{
    if (const YAML::Node *pNode = node.FindValue("qsfpp_reset")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfpp_reset = op;
    }
    if (const YAML::Node *pNode = node.FindValue("qsfpp_mod_present")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfpp_mod_present = op;
    }
    if (const YAML::Node *pNode = node.FindValue("qsfpp_int")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfpp_int = op;
    }
    if (const YAML::Node *pNode = node.FindValue("qsfpp_lp_mode")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfpp_lp_mode = op;
    }
    if (const YAML::Node *pNode = node.FindValue("qsfpp_interrupt")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.qsfpp_interrupt = op;
    }
}

static void operator >> (const YAML::Node &node, YamlSfpModuleSignals &signals)
{
    if (const YAML::Node *pNode = node.FindValue("sfpp_tx_disable")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.sfpp_tx_disable = op;
    }
    if (const YAML::Node *pNode = node.FindValue("sfpp_tx_fault")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.sfpp_tx_fault = op;
    }
    if (const YAML::Node *pNode = node.FindValue("sfpp_rx_loss")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.sfpp_rx_loss = op;
    }
    if (const YAML::Node *pNode = node.FindValue("sfpp_mod_present")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.sfpp_mod_present = op;
    }
    if (const YAML::Node *pNode = node.FindValue("sfpp_interrupt")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

        *pNode >> *op;

        signals.sfpp_interrupt = op;
    }
}

static void operator >> (const YAML::Node &node, YamlPort &port)
{
    string str;

    node["name"] >> str;
    port.name = strdup(str.c_str());

    node["switch_device"] >> port.device;

    node["switch_device_port"] >> port.device_port;

    node["pluggable"] >> port.pluggable;

    node["connector"] >> str;
    port.connector = strdup(str.c_str());

    node["max_speed"] >> port.max_speed;

    vector<int> speeds;
    node["speeds"] >> speeds;
    port.speeds = (int **)malloc(sizeof(int *) * (speeds.size() + 1));
    port.speeds[speeds.size()] = NULL;

    for (size_t idx = 0; idx < speeds.size(); idx++) {
        port.speeds[idx] = (int *)malloc(sizeof(int));
        *port.speeds[idx] = speeds[idx];
    }

    vector<string> strs;

    node["capabilities"] >> strs;
    port.capabilities = (char **)malloc(sizeof(char *) * (strs.size() + 1));
    port.capabilities[strs.size()] = NULL;

    for (size_t idx = 0; idx < strs.size(); idx++) {
        port.capabilities[idx] = strdup(strs[idx].c_str());
    }

    strs.clear();

    node["subports"] >> strs;
    port.subports = (char **)malloc(sizeof(char *) * (strs.size() + 1));
    port.subports[strs.size()] = NULL;

    for (size_t idx = 0; idx < strs.size(); idx++) {
        port.subports[idx] = strdup(strs[idx].c_str());
    }

    strs.clear();

    node["supported_modules"] >> strs;
    port.supported_modules = (char **)malloc(sizeof(char *) * (strs.size() + 1));
    port.supported_modules[strs.size()] = NULL;

    for (size_t idx = 0; idx < strs.size(); idx++) {
        port.supported_modules[idx] = strdup(strs[idx].c_str());
    }

    strs.clear();

    if (port.pluggable) {
        node["module_eeprom"] >> str;
        port.module_eeprom = strdup(str.c_str());

        if (strcmp(port.connector, SFPP) == 0) {
            node["module_signals"] >> port.module_signals.sfp;
        } else if (strcmp(port.connector, QSFPP) == 0) {
            node["module_signals"] >> port.module_signals.qsfp;
        } else if (strcmp(port.connector, QSFP28) == 0) {
            node["module_signals"] >> port.module_signals.qsfp28;
        } else {
            memset(&port.module_signals, 0, sizeof(YamlModuleSignals));
        }
    }

    if (const YAML::Node *pName = node.FindValue("parent_port")) {
        node["parent_port"] >> str;
        port.parent_port = strdup(str.c_str());
    } else {
        port.parent_port = NULL;
    }

    if (const YAML::Node *pNode = node.FindValue("subport_number")) {
        node["subport_number"] >> port.subport_number;
    } else {
        port.subport_number = 0;
    }
}

static void operator >> (const YAML::Node &node, vector<YamlPort> &ports)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlPort port;
        node[i] >> port;
        ports.push_back(port);
    }
}

static void operator >> (const YAML::Node &node, YamlLedInfo &led_info)
{
    node["number_leds"] >> led_info.number_leds;
    node["number_types"] >> led_info.number_types;
}

static void operator >> (const YAML::Node &node, YamlLedTypeSettings
        &settings)
{
    string str;

    node["OFF"] >> str;
    settings.off = (unsigned char)strtol(str.c_str(), 0, 0);
    node["ON"] >> str;
    settings.on = (unsigned char)strtol(str.c_str(), 0, 0);
    node["FLASHING"] >> str;
    settings.flashing = (unsigned char)strtol(str.c_str(), 0, 0);
}

static void operator >> (const YAML::Node &node, YamlLedType &led_type)
{
    string str;
    node["type"] >> str;
    led_type.type = strdup(str.c_str());
    if (str == "loc") {
        led_type.value = LED_LOC;
    } else {
        led_type.value = LED_UNKNOWN;
    }

    node["settings"] >> led_type.settings;
}

static void operator >> (const YAML::Node &node, YamlLed &led)
{
    string str;

    i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

    node["name"] >> str;
    led.name = strdup(str.c_str());

    node["led_type"] >> str;
    led.type = strdup(str.c_str());

    node["led_access"] >> *op;
    led.led_access = op;
}

static void operator >> (const YAML::Node &node, vector<YamlLedType> &led_types)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlLedType led_type;
        node[i] >> led_type;
        led_types.push_back(led_type);
    }
}

static void operator >> (const YAML::Node &node, vector<YamlLed> &leds)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlLed led;
        node[i] >> led;
        leds.push_back(led);
    }
}

static void operator >> (const YAML::Node &node, YamlPsuInfo &psu_info)
{
    node["number_psus"] >> psu_info.number_psus;
    node["polling_period"] >> psu_info.polling_period;
}

static void operator >> (const YAML::Node &node, YamlPsu &psu)
{
    node["number"] >> psu.number;
    if (const YAML::Node *pNode = node.FindValue("psu_present")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));
        *pNode >> *op;
        psu.psu_present = op;
    } else {
        psu.psu_present = NULL;
    }
    if (const YAML::Node *pNode = node.FindValue("psu_input_ok")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));
        *pNode >> *op;
        psu.psu_input_ok = op;
    } else {
        psu.psu_input_ok = NULL;
    }
    if (const YAML::Node *pNode = node.FindValue("psu_output_ok")) {
        i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));
        *pNode >> *op;
        psu.psu_output_ok = op;
    } else {
        psu.psu_output_ok = NULL;
    }
}

static void operator >> (const YAML::Node &node, vector<YamlPsu> &psus)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlPsu psu;
        node[i] >> psu;
        psus.push_back(psu);
    }
}

static void operator >> (const YAML::Node &node, YamlFanSpeed &speed)
{
    string str;

    node >> str;

    if (str == "SLOW") {
        speed = SLOW;
    } else if (str == "NORMAL") {
        speed = NORMAL;
    } else if (str == "MEDIUM") {
        speed = MEDIUM;
    } else if (str == "FAST") {
        speed = FAST;
    } else if (str == "MAX") {
        speed = MAX;
    } else {
        throw "FanSpeed value incorrect: " + str;
    }
}

static void operator >> (const YAML::Node &node, YamlFanDirection &direction)
{
    string str;

    node >> str;
    if (str == "F2B") {
        direction = F2B;
    } else if (str == "B2F") {
        direction = B2F;
    } else if (str == "FIXED") {
        direction = FIXED;
    } else if (str == "SETTABLE") {
        direction = SETTABLE;
    } else {
        throw "FanDirection value is incorrect: " + str;
    }
}

static void operator >> (const YAML::Node &node, YamlFanControlType &control)
{
    string str;

    node >> str;

    if (str == "SINGLE") {
        control = SINGLE;
    } else if (str == "PER_FAN") {
        control = PER_FAN;
    } else {
        throw "FanControlType value is incorrect: " + str;
    }
}

static void operator >> (const YAML::Node &node, YamlLedValues &values)
{
    string str;

    node["OFF"] >> str;
    values.off = strtol(str.c_str(), 0, 0);
    node["GOOD"] >> str;
    values.good = strtol(str.c_str(), 0, 0);
    node["FAULT"] >> str;
    values.fault = strtol(str.c_str(), 0, 0);
}

static void operator >> (const YAML::Node &node, YamlDirectionValues &values)
{
    string str;

    node["F2B"] >> str;
    values.f2b = strtol(str.c_str(), 0, 0);

    node["B2F"] >> str;
    values.b2f = strtol(str.c_str(), 0, 0);
}

static void operator >> (const YAML::Node &node, YamlSpeedSettings &settings)
{
    string str;

    node["SLOW"] >> str;
    settings.slow = (unsigned char)strtol(str.c_str(), 0, 0);
    node["NORMAL"] >> str;
    settings.normal = (unsigned char)strtol(str.c_str(), 0, 0);
    node["MEDIUM"] >> str;
    settings.medium = (unsigned char)strtol(str.c_str(), 0, 0);
    node["FAST"] >> str;
    settings.fast = (unsigned char)strtol(str.c_str(), 0, 0);
    node["MAX"] >> str;
    settings.max = (unsigned char)strtol(str.c_str(), 0, 0);
}

static void operator >> (const YAML::Node &node, YamlFanInfo &info)
{
    i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

    node["number_fan_frus"] >> info.number_fan_frus;
    node["fan_speed_control_type"] >> info.fan_speed_control_type;
    node["fan_speed_control"] >> *op;
    info.fan_speed_control = op;
    node["fan_speed_min"] >> info.fan_speed_min;
    node["fan_speed_settings"] >> info.fan_speed_settings;
    node["fan_direction"] >> info.direction;

    if (const YAML::Node *pNode = node.FindValue("fan_direction_control")) {
        op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));
        *pNode >> *op;
        info.fan_direction_control = op;
    }

    info.direction_control_values.f2b = 0x0;
    info.direction_control_values.b2f = 0x1;

    if (const YAML::Node *pNode = node.FindValue("fan_direction_control_values")) {
        *pNode >> info.direction_control_values;
    }

    node["fan_direction_values"] >> info.direction_values;
    node["fan_speed_multiplier"] >> info.fan_speed_multiplier;
    node["fan_led_values"] >> info.fan_led_values;
}

static void operator >> (const YAML::Node &node, YamlFan &fan)
{
    string str;
    i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

    node["name"] >> str;
    fan.name = strdup(str.c_str());
    node["fault"] >> *op;
    fan.fan_fault = op;

    op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));
    node["speed"] >> *op;
    fan.fan_speed = op;
}

static void operator >> (const YAML::Node &node, vector<YamlFan> &fans)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlFan fan;
        node[i] >> fan;
        fans.push_back(fan);
    }
}

static void operator >> (const YAML::Node &node, YamlFanFru &fan_fru)
{
    i2c_bit_op *op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));

    node["number"] >> fan_fru.number;
    node["fan_leds"] >> *op;
    fan_fru.fan_leds = op;

    op = (i2c_bit_op *)malloc(sizeof(i2c_bit_op));
    node["fan_direction_detect"] >> *op;
    fan_fru.fan_direction_detect = op;

    vector<YamlFan> fans;

    node["fans"] >> fans;

    fan_fru.fans = (YamlFan **)calloc(sizeof(YamlFan *), fans.size() + 1);

    for (size_t idx = 0; idx < fans.size(); idx++) {
        fan_fru.fans[idx] = (YamlFan *)malloc(sizeof(YamlFan));
        *fan_fru.fans[idx] = fans[idx];
    }
}

static void operator >> (const YAML::Node &node, vector<YamlFanFru> &fans)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlFanFru fan;
        node[i] >> fan;
        fans.push_back(fan);
    }
}

static void operator >> (const YAML::Node &node, YamlFruInfo &fru_info)
{
    string str;
    node["device_version"] >> fru_info.device_version;
    node["num_mac"] >> fru_info.num_macs;
    node["country_code"] >> str;
    fru_info.country_code = strdup(str.c_str());
    node["diag_version"] >> str;
    fru_info.diag_version = strdup(str.c_str());
    node["label_revision"] >> str;
    fru_info.label_revision = strdup(str.c_str());
    node["mac_base"] >> str;
    fru_info.base_mac_address = strdup(str.c_str());
    node["manufacture_date"] >> str;
    fru_info.manufacture_date = strdup(str.c_str());
    node["manufacturer"] >> str;
    fru_info.manufacturer = strdup(str.c_str());
    node["onie_version"] >> str;
    fru_info.onie_version = strdup(str.c_str());
    node["part_number"] >> str;
    fru_info.part_number = strdup(str.c_str());
    node["platform_name"] >> str;
    fru_info.platform_name = strdup(str.c_str());
    node["product_name"] >> str;
    fru_info.product_name = strdup(str.c_str());
    node["serial_number"] >> str;
    fru_info.serial_number = strdup(str.c_str());
    node["service_tag"] >> str;
    fru_info.service_tag = strdup(str.c_str());
    node["vendor"] >> str;
    fru_info.vendor = strdup(str.c_str());
}

static void operator >> (const YAML::Node &node, YamlFile &file)
{
    string str;
    node["name"] >> str;
    file.name = strdup(str.c_str());
    node["filename"] >> str;
    file.filename = strdup(str.c_str());
}

static void operator >> (const YAML::Node &node, map<string, YamlFile> &files)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlFile file;
        node[i] >> file;
        files[file.name] = file;
    }
}

static void operator >> (const YAML::Node &node, YamlBus &bus)
{
    string str;
    node["name"] >> str;
    bus.name = strdup(str.c_str());
    node["dev_name"] >> str;
    bus.devname = strdup(str.c_str());
    node["smbus"] >> bus.smbus;
}

static void operator >> (const YAML::Node &node, map<string, YamlBus> &buses)
{
    for (size_t i = 0; i < node.size(); i++) {
        YamlBus bus;
        node[i] >> bus;
        buses[bus.name] = bus;
    }
}

void
init_info_fields(YamlSubsystem *sub)
{
    YamlPsuInfo             psu_info;

    YamlLedInfo             led_info;

    // YamlSubsysInfo
    sub->subsys_info.info = NULL;

    // YamlThermalInfo
    sub->thermal.number_sensors = 0;
    sub->thermal.polling_period = 0;
    sub->thermal.auto_shutdown = 0;

    // YamlPortInfo
    sub->port_info.number_ports = 0;
    sub->port_info.max_port_speed = 0;
    sub->port_info.max_transmission_unit = 0;
    sub->port_info.max_lag_count = 0;
    sub->port_info.max_lag_member_count = 0;
    sub->port_info.l3_port_requires_internal_vlan = 0;

    // YamlFanInfo
    sub->fan_info.number_fan_frus = 0;
    sub->fan_info.fans_per_fru = 0;
    sub->fan_info.fan_speed_control_type = SINGLE;
    sub->fan_info.fan_speed_control = NULL;
    sub->fan_info.fan_speed_min = NORMAL;
    sub->fan_info.fan_speed_settings.slow = '\0';
    sub->fan_info.fan_speed_settings.normal = '\0';
    sub->fan_info.fan_speed_settings.medium = '\0';
    sub->fan_info.fan_speed_settings.fast = '\0';
    sub->fan_info.fan_speed_settings.max = '\0';
    sub->fan_info.direction = F2B;
    sub->fan_info.fan_direction_control = NULL;
    sub->fan_info.direction_values.f2b = '\0';
    sub->fan_info.direction_values.b2f = '\0';
    sub->fan_info.direction_control_values.f2b = '\0';
    sub->fan_info.direction_control_values.b2f = '\0';
    sub->fan_info.fan_speed_multiplier = 0;
    sub->fan_info.fan_led_values.off = '\0';
    sub->fan_info.fan_led_values.good = '\0';
    sub->fan_info.fan_led_values.fault = '\0';

    // YamlPsuInfo
    sub->psu_info.number_psus = 0;
    sub->psu_info.polling_period = 0;

    // YamlLedInfo
    sub->led_info.number_leds = 0;
    sub->led_info.number_types = 0;

    // YamlFruInfo
    sub->fru_info.country_code = '\0';
    sub->fru_info.device_version = 0;
    sub->fru_info.diag_version = '\0';
    sub->fru_info.label_revision = '\0';
    sub->fru_info.manufacture_date = '\0';
    sub->fru_info.manufacturer = '\0';
    sub->fru_info.num_macs = 0;
    sub->fru_info.onie_version = '\0';
    sub->fru_info.part_number = '\0';
    sub->fru_info.platform_name = '\0';
    sub->fru_info.product_name = '\0';
    sub->fru_info.serial_number = '\0';
    sub->fru_info.service_tag = '\0';
    sub->fru_info.vendor = '\0';
    sub->fru_info.base_mac_address = '\0';
}

extern "C" const YamlLedType *
yaml_get_led_type(YamlConfigHandle handle, const char *subsyst,
                                    unsigned int idx)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    if ((size_t)idx >= sub->led_types.size()) {
        return(NULL);
    }
    return(&sub->led_types[idx]);
}

extern "C" const YamlLedInfo *
yaml_get_led_info(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    return(&sub->led_info);
}

extern "C" const YamlLed *
yaml_get_led(YamlConfigHandle handle, const char *subsyst, unsigned int idx)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    if ((size_t)idx >= sub->leds.size()) {
        return(NULL);
    }
    return(&sub->leds[idx]);
}

extern "C" int
yaml_get_led_type_count(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    return(sub->led_types.size());
}

extern "C" int
yaml_get_led_count(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    return(sub->leds.size());
}

extern "C" const YamlPsuInfo *
yaml_get_psu_info(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    return(&sub->psu_info);
}

extern "C" const YamlFruInfo *
yaml_get_fru_info(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    return(&sub->fru_info);
}

extern "C" const YamlPsu *
yaml_get_psu(YamlConfigHandle handle, const char *subsyst, unsigned int idx)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    if ((size_t)idx >= sub->psus.size()) {
        return(NULL);
    }
    return(&sub->psus[idx]);
}

extern "C" int
yaml_get_psu_count(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    return(sub->psus.size());
}

extern "C" const YamlDevice *
yaml_find_device(YamlConfigHandle handle, const char *subsyst, const char *dev_name)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    string name = dev_name;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    try {
        return(&sub->device_map.at(name));
    } catch(...) {
        return(NULL);
    }
}

extern "C" const YamlSensor *
yaml_get_sensor(YamlConfigHandle handle, const char *subsyst, unsigned int idx)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    if ((size_t)idx >= sub->sensors.size()) {
        return(NULL);
    }
    return(&sub->sensors[idx]);
}

extern "C" const YamlPort *
yaml_get_port(YamlConfigHandle handle, const char *subsyst, unsigned int idx)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    if ((size_t)idx >= sub->ports.size()) {
        return(NULL);
    }
    return(&sub->ports[idx]);
}

extern "C" int
yaml_get_port_count(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    return(sub->ports.size());
}

extern "C" YamlPortInfo *
yaml_get_port_info(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    return(&sub->port_info);
}

extern "C" int
yaml_get_sensor_count(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    return(sub->sensors.size());
}

extern "C" const YamlFanFru *
yaml_get_fan_fru(YamlConfigHandle handle, const char *subsyst, unsigned int idx)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    if ((size_t)idx >= sub->fan_frus.size()) {
        return(NULL);
    }
    return(&sub->fan_frus[idx]);
}

extern "C" int
yaml_get_fan_fru_count(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    return(sub->fan_frus.size());
}

extern "C" const YamlFanInfo *
yaml_get_fan_info(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    return(&sub->fan_info);
}

extern "C" const YamlThermalInfo *
yaml_get_thermal_info(YamlConfigHandle handle, const char *subsyst)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    return &sub->thermal;
}

extern "C" int
yaml_parse_manifest(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;

    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // The name of the manifest file is fixed.
    string file_name = sub->dir_name + YAML_MANIFEST_FILENAME;

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    string str;
    doc["subsystem_info"] >> str;
    sub->subsys_info.info = strdup(str.c_str());

    try {
        doc["files"] >> sub->file_map;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" int
yaml_parse_buses(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;

    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the devices file
    yfile = yaml_find_file(handle, subsyst, YAML_DEVICES_NAME);

    if (yfile == NULL) {
        return(0);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["buses"] >> sub->bus_map;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" int
yaml_parse_devices(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the devices file
    yfile = yaml_find_file(handle, subsyst, YAML_DEVICES_NAME);

    if (yfile == NULL) {
        return(0);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["devices"] >> sub->device_map;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["init"] >> sub->init_ops;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }
    // also parse the buses, which must be in the same file
    return yaml_parse_buses(handle, subsyst);
}

extern "C" int
yaml_parse_thermal(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the thermal file
    yfile = yaml_find_file(handle, subsyst, YAML_THERMAL_NAME);

    if (yfile == NULL) {
        return(0);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["thermal_info"]["polling_period"] >> sub->thermal.polling_period;
        doc["thermal_info"]["auto_shutdown"] >> sub->thermal.auto_shutdown;
        doc["sensors"] >> sub->sensors;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" int
yaml_parse_ports(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the ports file
    yfile = yaml_find_file(handle, subsyst, YAML_PORTS_NAME);

    if (yfile == NULL) {
        return(0);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["port_info"] >> sub->port_info;

        doc["ports"] >> sub->ports;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" int
yaml_parse_fans(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the fans file
    yfile = yaml_find_file(handle, subsyst, YAML_FANS_NAME);

    if (yfile == NULL) {
        return(0);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["fan_info"] >> sub->fan_info;
        doc["fan_frus"] >> sub->fan_frus;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" int
yaml_parse_psus(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the power file
    yfile = yaml_find_file(handle, subsyst, YAML_POWER_NAME);

    if (yfile == NULL) {
        return(0);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["power_info"] >> sub->psu_info;
        doc["psus"] >> sub->psus;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" int
yaml_parse_leds(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the leds file
    yfile = yaml_find_file(handle, subsyst, YAML_LEDS_NAME);

    if (yfile == NULL) {
        return(0);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return -1;
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["led_info"] >> sub->led_info;
        doc["led_types"] >> sub->led_types;
        doc["leds"] >> sub->leds;
    } catch (YAML::RepresentationException &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" int
yaml_parse_fru(YamlConfigHandle handle, const char *subsyst)
{
    YAML::Node doc;
    const YamlFile *yfile = NULL;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    YamlSubsystem *sub = NULL;
    string sub_str = subsyst;
    try {
        sub = priv_hand->subsystem_map.at(sub_str);
    } catch(...) {
        return(-1);
    }

    // Get the name for the ports file
    yfile = yaml_find_file(handle, subsyst, YAML_FRU_NAME);

    if (yfile == NULL) {
        return(-1);
    }

    string file_name = sub->dir_name + string(yfile->filename);

    ifstream fin(file_name.c_str());
    if (fin.fail()) {
        return(-1);
    }

    try {
        YAML::Parser parser(fin);
        parser.GetNextDocument(doc);
    } catch (YAML::ParserException &pe) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    try {
        doc["fru_info"] >> sub->fru_info;
    } catch (YAML::KeyNotFound &re) {
        return(-1);
    } catch (...) {
        return(-1);
    }

    return(0);
}

extern "C" YamlConfigHandle
yaml_new_config_handle(void)
{
    YamlConfigHandlePrivate *handle = new YamlConfigHandlePrivate;

    return((YamlConfigHandle)handle);
}

extern "C" int
yaml_add_subsystem(YamlConfigHandle handle, const char *subsyst,
                                                const char *dir_name)
{
    string sub_str = subsyst;
    YamlConfigHandlePrivate *priv_hand = (YamlConfigHandlePrivate *)handle;

    if (priv_hand->subsystem_map.find(sub_str) !=
                                priv_hand->subsystem_map.end()) {
        return(-1);
    }

    YamlSubsystem *sub = new YamlSubsystem;

    priv_hand->subsystem_map[sub_str] = sub;

    init_info_fields(sub);

    string str = dir_name;
    str = str + '/';
    sub->dir_name = strdup(str.c_str());

    return (yaml_parse_manifest(handle, subsyst));
}

extern "C" const YamlFile *
yaml_find_file(YamlConfigHandle handle, const char *subsyst, const char *name)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    string file_name = name;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    try {
        return(&sub->file_map.at(file_name));
    } catch(...) {
        return(NULL);
    }
}

extern "C" const YamlBus *
yaml_find_bus(YamlConfigHandle handle, const char *subsyst, const char *name)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;

    string busname = name;
    string sub_str = subsyst;

    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return(NULL);
    }

    try {
        return(&sub->bus_map.at(busname));
    } catch(...) {
        return(NULL);
    }
}

extern "C" int
yaml_add_device(YamlConfigHandle handle, const char *subsystem, const char *dev_name, const YamlDevice *device)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string name = dev_name;
    string sub_str = subsystem;

    YamlSubsystem *sub = NULL;
    const YamlDevice *old_device = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return -1;
    }

    try {
        old_device = &sub->device_map.at(name);
    } catch(...) {
        old_device = NULL;
    }

    if (old_device != NULL) {
        return -1;
    }

    sub->device_map[name] = *device;

    return 0;
}

extern "C" int
yaml_init_devices(YamlConfigHandle handle, const char *subsystem)
{
    YamlConfigHandlePrivate *priv_handle = (YamlConfigHandlePrivate *)handle;
    string sub_str = subsystem;
    YamlSubsystem *sub = NULL;

    try {
        sub = priv_handle->subsystem_map.at(sub_str);
    } catch(...) {
        return -1;
    }

    for (size_t idx = 0; idx < sub->init_ops.size(); idx++) {
        int rc;
        i2c_op **ops = (i2c_op **)malloc(sizeof(i2c_op *) * 2);
        i2c_op op = sub->init_ops.at(idx);
        const YamlDevice *device;
        ops[0] = &op;
        ops[1] = NULL;

        device = yaml_find_device(handle, subsystem, op.device);

        rc = i2c_execute(handle, subsystem, device, ops);

        free(ops);
    }

    return (0);
}
