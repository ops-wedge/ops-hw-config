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

#ifndef _I2C_H_
#define _I2C_H_

#ifndef __cplusplus
#include <stdbool.h>
#endif

#define WRITE   true
#define READ    false

typedef struct {
    bool            direction;
    char            *device;
    int             byte_count;
    bool            set_register;
    unsigned char   register_address;
    unsigned char   *data;
    bool            negative_polarity;
} i2c_op;

typedef struct {
    char            *device;
    unsigned char   register_address;
    unsigned char   register_size;      // 1, 2, or 4 byte register
    unsigned char   bit_mask;
    bool            negative_polarity;
} i2c_bit_op;

#endif
