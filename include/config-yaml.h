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

/************************************************************************//**
 * @defgroup config_yaml OpenSwitch H/W Configuration and I2C Library
 * This library provides functions to parse the H/W description files
 * written in YAML language for OpenSwitch subsystems, as well as functions that
 * access i2c devices based on the contents of the H/W description files.
 *
 * @{
 *
 * @defgroup config_yaml_public Public Interface
 * Public API for config-yaml library.
 *
 * - Configuration: APIs to parse and access information in the H/W
 *                  description files.
 * - I2C: API for reading and writing i2c devices based on information
 *        found in the H/W description files.
 *
 * @{
 *
 * @file
 * Header for config_yaml library.
 ***************************************************************************/


#ifndef _CONFIG_YAML_H_
#define _CONFIG_YAML_H_

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"

#define SFPP    "SFP_PLUS"      /*!< String to identify SFP+ modules */
#define QSFPP   "QSFP_PLUS"     /*!< String to identify QSFP+ modules */
#define QSFP28  "QSFP28"        /*!< String to identify QSFP28 modules */

#define YAML_MANIFEST_FILENAME "manifest.yaml" /*!< Filename of manifest file */

#define YAML_MANIFEST_NAME "manifest" /*!< Name to identify manifest file */
#define YAML_DEVICES_NAME "devices"   /*!< Name to identify devices file */
#define YAML_FANS_NAME "fans"         /*!< Name to identify fans file */
#define YAML_LEDS_NAME "leds"         /*!< Name to identify leds file */
#define YAML_PORTS_NAME "ports"       /*!< Name to identify ports file */
#define YAML_POWER_NAME "power"       /*!< Name to identify power file */
#define YAML_THERMAL_NAME "thermal"   /*!< Name to identify thermal file */
#define YAML_FRU_NAME "fru"           /*!< Name to identify fru file */

/************************************************************************//**
 * STRUCT that contains the content of the subsystem_info portion of the
 *    manifest.yaml file.
 ***************************************************************************/
typedef struct {
    char    *info;      /*!< subsystem_info contents */
} YamlSubsysInfo;

/************************************************************************//**
 * STRUCT that contains the content of the file section of the manifest.yaml
 *    file.
 ***************************************************************************/
typedef struct {
    char    *name;      /*!< name identify for the yaml file */
    char    *filename;  /*!< filename for the yaml file */
} YamlFile;

/************************************************************************//**
 * STRUCT that contains the content of the device section of the
 *    devices.yaml file.
 ***************************************************************************/
typedef struct {
    char    *name;      /*!< Name identifier for the device */
    char    *bus;       /*!< Bus this device is on */
    char    *dev_type;  /*!< Device type identifier */
    int     address;    /*!< Address for the device on the bus */
    i2c_op  **pre;      /*!< i2c pre operation */
    i2c_op  **post;     /*!< i2c post operation */
} YamlDevice;

/************************************************************************//**
 * STRUCT that contains the content of the bus section of the devices.yaml
 *    file.
 ***************************************************************************/
typedef struct {
    char    *name;      /*!< Name identifier for the bus */
    char    *devname;   /*!< /dev/<name> used to access the bus */
    bool    smbus;      /*!< True if the bus is an SMBUS */
} YamlBus;

/************************************************************************//**
 * STRUCT that contains the content of the alarm_thresholds section of
 *    the sensors section of the thermal.yaml file.
 ***************************************************************************/
typedef struct {
    float emergency_on;     /*!< Min temp to set emergency level */
    float emergency_off;    /*!< Max temp to remove emergency level */
    float critical_on;      /*!< Min temp to set critical level */
    float critical_off;     /*!< Max temp to remove critical level */
    float max_on;           /*!< Min temp to set fan speed to max */
    float max_off;          /*!< Max temp to drop max fan speed */
    float min;              /*!< Max temp to set fan speed to min */
    float low_crit;         /*!< Max temp to set low critical level */
} YamlAlarmThresholds;

/************************************************************************//**
 * STRUCT that contains the content of the fan_thresholds section of the
 *    sensors section of the thermal.yaml file.
 ***************************************************************************/
typedef struct {
    float max_on;           /*!< Min temp to set fan speed to max */
    float max_off;          /*!< Max temp to drop fan speed below max */
    float fast_on;          /*!< Min temp to set fan speed to fast */
    float fast_off;         /*!< Max temp to drop fan speed below fast */
    float medium_on;        /*!< Min temp to set fan speed to medium */
    float medium_off;       /*!< Max temp to drop fan speed below medium */
} YamlFanThresholds;

/************************************************************************//**
 * STRUCT that contains the content of the sensors section of the
 *    thermal.yaml file.
 ***************************************************************************/
typedef struct {
    int                 number;     /*!< Sensor identifier */
    char                *location;  /*!< Sensor location description */
    char                *device;    /*!< Device name for the sensor */
    char                *type;      /*!< Sensor type */
    YamlAlarmThresholds alarm_thresholds; /*!< YamlAlarmThresholds struct */
    YamlFanThresholds   fan_thresholds;   /*!< YamlFanThresholds struct */
} YamlSensor;

/************************************************************************//**
 * STRUCT that contains the content of the thermal_info section of the
 *    thermal.yaml file.
 ***************************************************************************/
typedef struct {
    int     number_sensors; /*!< Number of sensors in this subsystem */
    int     polling_period; /*!< Polling period in miliseconds */
    bool    auto_shutdown;  /*!< True if auto shutdown for emergency level */
} YamlThermalInfo;

/************************************************************************//**
 * STRUCT that contains the content of the port_info section of the
 *    ports.yaml file.
 ***************************************************************************/
typedef struct {
    int     number_ports;           /*!< Number of ports in this subsystem */
    int     max_port_speed;         /*!< Max port speed in Mbits/sec */
    int     max_transmission_unit;  /*!< Max MTU */
    int     max_lag_count;          /*!< Max support lags */
    int     max_lag_member_count;   /*!< Max members allowed in a lag */
    bool    l3_port_requires_internal_vlan; /*!< L3 port requires internal VLAN */
} YamlPortInfo;

/************************************************************************//**
 * STRUCT that contains the content of the module_signals section for SFP+
 *    modules in the ports section of the ports.yaml file.
 ***************************************************************************/
typedef struct {
    i2c_bit_op  *sfpp_tx_disable;   /*!< i2c op to enable/disable tx */
    i2c_bit_op  *sfpp_tx_fault;     /*!< i2c op to determine if tx fault */
    i2c_bit_op  *sfpp_rx_loss;      /*!< i2c op to determine if rx loss */
    i2c_bit_op  *sfpp_mod_present;  /*!< i2c op to determine module presence */
    i2c_bit_op  *sfpp_interrupt;    /*!< i2c op for sfp+ interrupt */
} YamlSfpModuleSignals;

/************************************************************************//**
 * STRUCT that contains the content of the module_signals section for QSFP+
 *    modules in the ports section of the ports.yaml file.
 ***************************************************************************/
typedef struct {
    i2c_bit_op  *qsfpp_reset;       /*!< i2c op to reset the module */
    i2c_bit_op  *qsfpp_mod_present; /*!< i2c op to determine module presence */
    i2c_bit_op  *qsfpp_int;         /*!< interrupt status,
                                        0: generates interrupt when present
                                        1: no interrupt */
    i2c_bit_op  *qsfpp_lp_mode;     /*!< low power mode,
                                        0: set LPMODE in High Power
                                        1: set LPMODE in Low Power */
    i2c_bit_op  *qsfpp_interrupt;   /*!< i2c op for qsfp+ interrupt */
} YamlQsfpModuleSignals;

/************************************************************************//**
 * STRUCT that contains the content of the module_signals section for QSFP28
 *    modules in the ports section of the ports.yaml file.
 ***************************************************************************/
typedef struct {
    i2c_bit_op  *qsfp28p_reset;          /*!< i2c op to reset the module */
    i2c_bit_op  *qsfp28p_mod_present;    /*!< i2c op to determine module presence */
    i2c_bit_op  *qsfp28p_interrupt;      /*!< i2c op to determine interrupt status */
    i2c_bit_op  *qsfp28p_interrupt_mask; /*!< i2c op to mask incoming interrupts */
} YamlQsfp28ModuleSignals;

/************************************************************************//**
 * UNION for module operation information for either SFP+ or QSFP+ or QSFP28
 *    modules.
 ***************************************************************************/
typedef union {
    YamlSfpModuleSignals     sfp;     /*!< SFP+ op commands */
    YamlQsfpModuleSignals    qsfp;    /*!< QSFP+ op commands */
    YamlQsfp28ModuleSignals  qsfp28;  /*!< QSFP28 op commands */
} YamlModuleSignals;

/************************************************************************//**
 * STRUCT that contains the content of the ports section of the ports.yaml
 *    file.
 ***************************************************************************/
typedef struct {
    char              *name;       /*!< Name identifier for the port */
    bool              pluggable;   /*!< True if supports pluggable modules */
    char              *connector;  /*!< Connector type, e.g. SFP, QSFP */
    int               max_speed;   /*!< Max speed in Mb/sec */
    int               **speeds;    /*!< List of supports speeds in Mb/sec */
    unsigned int      device;      /*!< ID of switch port is connected to */
    unsigned int      device_port; /*!< Dev ID on the switch for this port */
    char              **subports; /*!< List of subports if port is splittable */
    char              **capabilities; /*!< Port capabilities */
    char              **supported_modules; /*!< List of supported modules */
    char              *module_eeprom; /*!< i2c device ID for module EEPROM */
    char              *parent_port;   /*!< parent port if subport */
    YamlModuleSignals module_signals; /*!< i2c ops for this module */
    unsigned int      subport_number; /*!< sub id of a subport */
} YamlPort;

/************************************************************************//**
 * ENUM for control type for fans in the fan_info section of the fans.yaml
 *    file.
 ***************************************************************************/
typedef enum {
    SINGLE,         /*!< Commands apply to all fans in this fan FRU */
    PER_FAN         /*!< Commands apply per fan in this fan FRU */
} YamlFanControlType;

/************************************************************************//**
 * ENUM defining possible settable fan speeds for the fan_info section of
 *    the fans.yaml file.
 ***************************************************************************/
typedef enum {
    SLOW,           /*!< slow speed, e.g.,   25% */
    NORMAL,         /*!< normal speed, e.g., 40% */
    MEDIUM,         /*!< medium speed, e.g., 65% */
    FAST,           /*!< fast speed, e.g.,   80% */
    MAX             /*!< max speed, e.g.,   100% */
} YamlFanSpeed;

/************************************************************************//**
 * ENUM defining possible values for airflow direction from the fan_info
 *    section of the fans.yaml file.
 ***************************************************************************/
typedef enum {
    F2B,            /*!< Front to Back */
    B2F,            /*!< Back to Front */
    FIXED,          /*!< fixed, that is, it can't be changed */
    SETTABLE        /* OPS_TODO: this doesn't make sense to me */
} YamlFanDirection;     /* doesn't B2F or F2B mean FIXED? */

/************************************************************************//**
 * STRUCT that contains the content of the fan_speed_settings field from
 *    the fan_info section of the fans.yaml file.
 ***************************************************************************/
typedef struct {
    unsigned char slow;     /*!< YamlFanSpeed SLOW */
    unsigned char normal;   /*!< YamlFanSpeed NORMAL */
    unsigned char medium;   /*!< YamlFanSpeed MEDIUM */
    unsigned char fast;     /*!< YamlFanSpeed FAST */
    unsigned char max;      /*!< YamlFanSpeed MAX */
} YamlSpeedSettings;

/************************************************************************//**
 * STRUCT that contains the content of the fan_direction_values field from
 *    the fan_info section of the fans.yaml file.
 ***************************************************************************/
typedef struct {
    unsigned char f2b;      /*!< Fan airflow is Front to Back */
    unsigned char b2f;      /*!< Fan airflow is Back to Front */
} YamlDirectionValues;

/************************************************************************//**
 * STRUCT that contains the content of the fan_led_values field from the
 *    fan_info section of the fans.yaml file.
 ***************************************************************************/
typedef struct {
    unsigned char off;      /*!< Fan LED value of off */
    unsigned char good;     /*!< Fan LED value of good */
    unsigned char fault;    /*!< Fan LED value of fault */
} YamlLedValues;

/************************************************************************//**
 * STRUCT that contains the content of the fan_info section of the fans.yaml
 *    file.
 ***************************************************************************/
typedef struct {
    int                 number_fan_frus;    /*!< Number of Fan FRUs(not fans) */
    int                 fans_per_fru;       /*!< Number of Fans per FRU */
    YamlFanControlType  fan_speed_control_type; /*!< YamlFanControlType */
    i2c_bit_op          *fan_speed_control; /*!< Op to set fan speed */
    YamlFanSpeed        fan_speed_min;      /*!< Minimum fan speed */
    YamlSpeedSettings   fan_speed_settings; /*!< Values for each speed  */
    YamlFanDirection    direction;          /*!< Airflow direction */
    i2c_bit_op          *fan_direction_control; /*!< dir ctrl op values */
    YamlDirectionValues direction_values;   /*!< op values to set direction */
    YamlDirectionValues direction_control_values; /*!< dir ctrl values */
    int                 fan_speed_multiplier; /*!< multiplier to set speed */
    YamlLedValues       fan_led_values;     /*!< Fan LED values */
} YamlFanInfo;

/************************************************************************//**
 * STRUCT that contains the content of the fans section of the fan_frus
 *    section of the fans.yaml file.
 ***************************************************************************/
typedef struct {
    char        *name;      /*!< Name identified of the Fan */
    i2c_bit_op  *fan_fault; /*!< op values for accessing fan fault */
    i2c_bit_op  *fan_speed; /*!< op values for accessing fan speed */
} YamlFan;

/************************************************************************//**
 * STRUCT that contains the content of the fan_frus section of the fans.yaml
 *    file.
 ***************************************************************************/
typedef struct {
    int         number;     /*!< Fan FRU identifier number */
    YamlFan     **fans;     /*!< YamlFan pointers for fans in the FRU */
    i2c_bit_op  *fan_leds;  /*!< op values to access the LEDs */
    i2c_bit_op  *fan_direction_detect; /*!< op values to set fan direction */
} YamlFanFru;

/************************************************************************//**
 * STRUCT that contains the content of the power_info section of the
 *    power.yaml file.
 ***************************************************************************/
typedef struct {
    int         number_psus;    /*!< Number of power supplies */
    int         polling_period; /*!< Polling period in milisecsonds */
} YamlPsuInfo;

/************************************************************************//**
 * STRUCT that contains the content of the PSUs (Power Supply Units) section
 * of the power.yaml file.
 ***************************************************************************/
typedef struct {
    int         number;          /*!< Power supply identifier number */
    i2c_bit_op  *psu_present;    /*!< op values for PSU presence */
    i2c_bit_op  *psu_input_ok;   /*!< op values for PSU input ok */
    i2c_bit_op  *psu_output_ok;  /*!< op values for PSU output ok */
} YamlPsu;

/************************************************************************//**
 * ENUM for the possible LED types
 ***************************************************************************/
typedef enum {
    LED_UNKNOWN,    /*!< Type unknown */
    LED_LOC         /*!< Type is "loc" for location */
} YamlLedTypeValue;

/************************************************************************//**
 * STRUCT that contains the content of the settings section of the led_types
 *    section of the leds.yaml file.
 ***************************************************************************/
typedef struct {
    unsigned char off;       /*!< Value for LED OFF setting */
    unsigned char on;        /*!< Value for LED ON setting */
    unsigned char flashing;  /*!< Value for LED FLASHING setting */
} YamlLedTypeSettings;

/************************************************************************//**
 * STRUCT that contains the content of the leds section of the leds.yaml file.
 ***************************************************************************/
typedef struct {
    char                *name;        /*!<  Name identifier for the LED */
    char                *type;        /*!<  LED type */
    i2c_bit_op          *led_access;  /*!<  op values to access the LED */
} YamlLed;

/************************************************************************//**
 * STRUCT that contains the content of the led_type section of the
 *    leds.yaml file.
 ***************************************************************************/
typedef struct {
    char   *type;                   /*!< Name identifier for LED type */
    YamlLedTypeValue    value;      /*!< ENUM value for this type */
    YamlLedTypeSettings  settings;  /*!< op settings for this type */
} YamlLedType;

/************************************************************************//**
 * STRUCT that contains the content of the led_info section of the
 *    leds.yaml file.
 ***************************************************************************/
typedef struct {
    int number_leds;        /*!< Number of LEDs */
    int number_types;       /*!< Number of LED types */
} YamlLedInfo;

/************************************************************************//**
 * STRUCT that contains the content of the fru_info section of the
 *    fru.yaml file.
 ***************************************************************************/
typedef struct {
   char *country_code;
   char device_version;
   char *diag_version;
   char *label_revision;
   char *base_mac_address;
   char *manufacture_date;
   char *manufacturer;
   int num_macs;
   char *onie_version;
   char *part_number;
   char *platform_name;
   char *product_name;
   char *serial_number;
   char *service_tag;
   char *vendor;
} YamlFruInfo;

/************************************************************************//**
 * TYPEDEF for the opaque Yaml config handle used for each call. The handle
 *    is returned by the yaml_new_config_handle() function.
 ***************************************************************************/
typedef void *YamlConfigHandle;

/************************************************************************//**
 * Returns a unique handle to identify a specific subsystem. There will be
 * one handle per subsystem.
 *
 * @return YamlConfigHandle
 ***************************************************************************/
extern YamlConfigHandle yaml_new_config_handle(void);

/************************************************************************//**
 * Adds a new subsystem to the config-yaml internal database. It finds
 * and parses the "manifest.yaml" file for this subsystem.
 *
 * @param[in] handle   :YamlConfigHandle for this subsystem
 * @param[in] subsyst  :Name of the subsystem
 * @param[in] dir_name :Full path to the directory that contains the YAML
 *                      files for this subsystem
 *
 * @return int :0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_add_subsystem(YamlConfigHandle handle, const char *subsyst, const char *dir_name);

/************************************************************************//**
 * Locates device information for a given device name
 *
 * @param[in] handle   :YamlConfigHandle for this subsystem
 * @param[in] subsyst  :Name of the subsystem
 * @param[in] dev_name :Name of the device to locate
 *
 * @return YamlDevice * on success, else NULL on failure
 ***************************************************************************/
extern const YamlDevice * yaml_find_device(YamlConfigHandle handle, const char *subsyst, const char *dev_name);

/************************************************************************//**
 * Add information for a new device
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsystem :Name of the subsystem
 * @param[in] dev_name  :Name of the device to add
 * @param[in] device    :ptr to the YamlDevice structure to add
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_add_device(YamlConfigHandle handle, const char *subsystem, const char *dev_name, const YamlDevice *device);

/************************************************************************//**
 * Initializes the i2c devices in a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsystem :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_init_devices(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to a specific sensor
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] idx       :Index for the specific sensor to retrieve
 *
 * @return YamlSensor * on success, else NULL on failure
 ***************************************************************************/
extern const YamlSensor * yaml_get_sensor(YamlConfigHandle handle, const char *subsyst, unsigned int idx);

/************************************************************************//**
 * Returns number of sensors in a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return number of sensors on success, else -1 on failure
 ***************************************************************************/
extern int yaml_get_sensor_count(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to thermal info for a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return YamlThermalInfo * on success, else NULL on failure
 ***************************************************************************/
extern const YamlThermalInfo * yaml_get_thermal_info(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to a specific port
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] idx       :Index for the specific port to retrieve
 *
 * @return YamlPort * on success, else NULL on failure
 ***************************************************************************/
extern const YamlPort * yaml_get_port(YamlConfigHandle handle, const char *subsyst, unsigned int idx);

/************************************************************************//**
 * Returns number of ports in a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return number of ports on success, else -1 on failure
 ***************************************************************************/
extern int yaml_get_port_count(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to generic port info for a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return YamlPortInfo * on success, else NULL on failure
 ***************************************************************************/
extern YamlPortInfo * yaml_get_port_info(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to a specific fan FRU
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] idx       :Index for the specific fan FRU to retrieve
 *
 * @return YamlFanFru * on success, else NULL on failure
 ***************************************************************************/
extern const YamlFanFru * yaml_get_fan_fru(YamlConfigHandle handle, const char *subsyst, unsigned int idx);

/************************************************************************//**
 * Returns number of fan FRUs in a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return number of fan FRUs on success, else -1 on failure
 ***************************************************************************/
extern int yaml_get_fan_fru_count(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to generic fan FRU info for a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return YamlFanInfo * on success, else NULL on failure
 ***************************************************************************/
extern const YamlFanInfo * yaml_get_fan_info(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the manifest.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_manifest(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the devices.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_devices(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the thermal.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_thermal(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the ports.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_ports(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the fans.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_fans(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the power.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_psus(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the leds.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_leds(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Performs the list of i2c commands for the specified i2c device
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] device    :Device to operate on
 * @param[in] ops       :List of i2c commands to send to the device
 *
 * @return 0 on success, else errno on failure
 ***************************************************************************/
extern int i2c_execute(YamlConfigHandle handle, const char *subsyst, const YamlDevice *device, i2c_op **ops);

/************************************************************************//**
 * Returns info for a specific bus
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] name      :Name of the bus to locate
 *
 * @return YamlBus * on success, else NULL on failure
 ***************************************************************************/
extern const YamlBus *yaml_find_bus(YamlConfigHandle handle, const char *subsyst, const char *name);

/************************************************************************//**
 * Returns info for a specific yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] name      :Name of the file (not filename)
 *
 * @return YamlFile * on success, else NULL on failure
 ***************************************************************************/
extern const YamlFile *yaml_find_file(YamlConfigHandle handle, const char *subsyst, const char *name);

/************************************************************************//**
 * Returns a pointer to the Power Supply Info in the power.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return YamlPsuInfo * on success, else NULL on failure
 ***************************************************************************/
extern const YamlPsuInfo *yaml_get_psu_info(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to a specific power supply
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] idx       :Index for the specific power supply to retrieve
 *
 * @return YamlPsu * on success, else NULL on failure
 ***************************************************************************/
extern const YamlPsu *yaml_get_psu(YamlConfigHandle handle, const char *subsyst, unsigned int idx);

/************************************************************************//**
 * Returns number of power supplies in a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return number of power supplies on success, else -1 on failure
 ***************************************************************************/
extern int yaml_get_psu_count(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to the LED Info in the leds.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return YamlLedInfo * on success, else NULL on failure
 ***************************************************************************/
extern const YamlLedInfo *yaml_get_led_info(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to a specific LED
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] idx       :Index for the specific LED to retrieve
 *
 * @return YamlLed * on success, else NULL on failure
 ***************************************************************************/
extern const YamlLed *yaml_get_led(YamlConfigHandle handle, const char *subsyst, unsigned int idx);

/************************************************************************//**
 * Returns a pointer to a specific LED type
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 * @param[in] idx       :Index for the specific LED type to retrieve
 *
 * @return YamlLedType * on success, else NULL on failure
 ***************************************************************************/
extern const YamlLedType *yaml_get_led_type(YamlConfigHandle handle, const char *subsyst, unsigned int idx);

/************************************************************************//**
 * Returns number of LEDs in a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return number of LEDs on success, else -1 on failure
 ***************************************************************************/
extern int yaml_get_led_count(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns number of LED Types in a subsystem
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return number of LED Types on success, else -1 on failure
 ***************************************************************************/
extern int yaml_get_led_type_count(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Parses and stores internally the information in the fru.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return 0 on success, else -1 on failure
 ***************************************************************************/
extern int yaml_parse_fru(YamlConfigHandle handle, const char *subsyst);

/************************************************************************//**
 * Returns a pointer to the Fru Info in the fru.yaml file
 *
 * @param[in] handle    :YamlConfigHandle for this subsystem
 * @param[in] subsyst   :Name of the subsystem
 *
 * @return YamlFruInfo * on success, else NULL on failure
 ***************************************************************************/
extern const YamlFruInfo *yaml_get_fru_info(YamlConfigHandle handle, const char *subsyst);

#ifdef __cplusplus
};
#endif

#endif
/** @} end of group config_yaml_public */
/** @} end of group config_yaml */
