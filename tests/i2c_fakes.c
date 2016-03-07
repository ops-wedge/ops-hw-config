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

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "../include/config-yaml.h"

extern int ops_cnt;

int
i2c_execute(
    YamlConfigHandle handle,
    const char *subsyst,
    const YamlDevice *dev,
    i2c_op **cmds)
{
    int i = 0;

    if ((handle != (YamlConfigHandle) NULL) &&
                        (cmds != NULL) && (dev != NULL)) {
        while (cmds[i] != NULL) {
            i++;
        }
        ops_cnt += i;

        printf("i2c_fake: %d i2c_op commands sent.\n", i);
        printf("i2c_fake: Returning success for subsystem %s.\n", subsyst);
        return 0;
    } else {
        return -1;
    }
}
