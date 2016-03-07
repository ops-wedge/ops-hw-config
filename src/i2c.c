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

#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

#include <linux/i2c-dev-user.h>

#include "config-yaml.h"

static int
count_ops(i2c_op **ops)
{
    int count = 0;

    while (ops[count] != NULL) {
        count++;
    }

    return(count);
}

static int
count_post(YamlConfigHandle *handle, const char *subsyst, const YamlDevice *dev)
{
    int count = 0;

    if (dev == NULL || dev->post == NULL) {
        return(0);
    }

    count += count_ops(dev->post);

    if (count != 0) {
        dev = yaml_find_device(handle, subsyst, dev->post[0]->device);
        count += count_post(handle, subsyst, dev);
    }

    return(count);
}

static int
count_pre(YamlConfigHandle *handle, const char *subsyst, const YamlDevice *dev)
{
    int count = 0;

    if (dev == NULL || dev->pre == NULL) {
        return(0);
    }

    count += count_ops(dev->pre);

    if (count != 0) {
        dev = yaml_find_device(handle, subsyst, dev->pre[0]->device);
        count += count_pre(handle, subsyst, dev);
    }

    return(count);
}

static int
add_post(
    YamlConfigHandle *handle,
    const char *subsyst,
    const YamlDevice *dev,
    i2c_op **cmds,
    int idx)
{
    int i;

    const YamlDevice *post_dev;

    if (dev->post == NULL || dev->post[0] == NULL) {
        return(idx);
    }

    post_dev = yaml_find_device(handle, subsyst, dev->post[0]->device);

    for (i = 0; dev->post[i] != NULL; i++) {
        cmds[idx] = dev->post[i];
        idx++;
    }

    idx = add_post(handle, subsyst, post_dev, cmds, idx);

    return(idx);
}

static int
add_pre(
    YamlConfigHandle *handle,
    const char *subsyst,
    const YamlDevice *dev,
    i2c_op **cmds,
    int idx)
{
    int i;

    const YamlDevice *pre_dev;

    if (dev->pre == NULL || dev->pre[0] == NULL) {
        return(idx);
    }

    pre_dev = yaml_find_device(handle, subsyst, dev->pre[0]->device);

    idx = add_pre(handle, subsyst, pre_dev, cmds, idx);

    for (i = 0; dev->pre[i] != NULL; i++) {
        cmds[idx] = dev->pre[i];
        idx++;
    }

    return(idx);
}

static int
add_cmds(i2c_op **ops, i2c_op **cmds, int idx)
{
    int i;

    for (i = 0; ops[i] != NULL; i++) {
        cmds[idx] = ops[i];
        idx++;
    }

    return(idx);
}

int
i2c_execute(
    YamlConfigHandle handle,
    const char *subsyst,
    const YamlDevice *dev,
    i2c_op **cmds)
{
    i2c_op **all_cmds;
    unsigned int count = 0;
    unsigned int idx = 0;
    char *bus_name;
    int fd = -1;
    unsigned int i;
    int rc;
    int final_rc;
    struct i2c_msg *msgbuf;
    struct i2c_rdwr_ioctl_data msgioctl;
    char *dev_name;

    if (dev == NULL || handle == NULL) {
        return EINVAL;
    }

    if (cmds == NULL || cmds[0] == NULL) {
        return EINVAL;
    }

    // OPS_TODO: need better i2c_op array functions to avoid
    // having to count operations before accumulating them.
    count += count_pre(handle, subsyst, dev);
    count += count_ops(cmds);
    count += count_post(handle, subsyst, dev);

    all_cmds = (i2c_op **)calloc(sizeof(i2c_op *), count);

    // OPS_TODO: need to look at bus to see if it crosses a subsystem boundary
    // and jump to the other subsystem (recursively) to pick up any pre and
    // post operations that may be required.
    idx = add_pre(handle, subsyst, dev, all_cmds, idx);
    idx = add_cmds(cmds, all_cmds, idx);
    idx = add_post(handle, subsyst, dev, all_cmds, idx);

    // verify that all operations are to the same bus
    // OPS_TODO: bus may change as we cross the boundary between subsystems

    dev = yaml_find_device(handle, subsyst, all_cmds[0]->device);

    if (dev == NULL) {
        return EINVAL;
    }

    bus_name = dev->bus;

    for (idx = 0; idx < count; idx++) {
        dev = yaml_find_device(handle, subsyst, all_cmds[idx]->device);
        if (strcmp(dev->bus, bus_name) != 0) {
            free(all_cmds);
            return EINVAL;
        }
    }

    const YamlBus *bus = yaml_find_bus(handle, subsyst, bus_name);

    if (bus == NULL) {
        free(all_cmds);
        return EINVAL;
    }

    dev_name = bus->devname;

    fd = open(dev_name, O_RDWR);

    if (fd < 0) {
        rc = errno;
        free(all_cmds);
        return rc;
    }

    rc = 0;
    final_rc = 0;

    if (!bus->smbus) {
        msgbuf = (struct i2c_msg *)calloc(sizeof(struct i2c_msg), count);

        msgioctl.nmsgs = count;
        msgioctl.msgs = msgbuf;

        for (i = 0; i < count; i++) {
            dev = yaml_find_device(handle, subsyst, all_cmds[i]->device);
            msgbuf[i].flags = all_cmds[i]->direction ? 0 : I2C_M_RD;
            msgbuf[i].len = all_cmds[i]->byte_count;
            msgbuf[i].addr = dev->address;
            msgbuf[i].buf = (char *)all_cmds[i]->data;
            idx++;
        }

        do {
            rc = ioctl(fd, I2C_RDWR, &msgioctl);
        } while (rc < 0 && EINTR == errno);

        if (rc < 0) {
            rc = errno;
            final_rc = rc;
        }

        free(msgbuf);
    } else {
        // This bus doesn't support i2c operations.
        flock(fd, LOCK_EX);

        for (idx = 0; idx < count; idx++) {
            i2c_op *cmd = all_cmds[idx];
            dev = yaml_find_device(handle, subsyst, cmd->device);

            rc = ioctl(fd, I2C_SLAVE, (long)dev->address);

            if (rc < 0) {
                rc = errno;
                final_rc = rc;
                continue;
            }

            if (cmd->direction) {
                // write
                if (1 == cmd->byte_count) {
                    long data;
                    data = (long)cmd->data[0];
                    rc = i2c_smbus_write_byte_data(
                            fd,
                            cmd->register_address,
                            data);
                    if (rc < 0) {
                        rc = errno;
                        final_rc = rc;
                        continue;
                    }
                } else if (2 == cmd->byte_count) {
                    long data;
                    data = (long)(*(unsigned short *)cmd->data);
                    rc = i2c_smbus_write_word_data(
                            fd,
                            cmd->register_address,
                            data);
                    if (rc < 0) {
                        rc = errno;
                        final_rc = rc;
                        continue;
                    }
                } else {
                    // NOT IMPLEMENTED
                    rc = EINVAL;
                    final_rc = rc;
                    continue;
                }
            } else {
                // read
                if (1 == cmd->byte_count) {
                    long data;
                    data = i2c_smbus_read_byte_data(
                                fd,
                                cmd->register_address);
                    if (data < 0) {
                        rc = errno;
                        final_rc = rc;
                        continue;
                    } else {
                        cmd->data[0] = (unsigned char)data;
                    }
                } else if (2 == cmd->byte_count) {
                    long data;
                    data = i2c_smbus_read_word_data(
                                fd,
                                cmd->register_address);
                    if (data < 0) {
                        rc = errno;
                        final_rc = rc;
                        continue;
                    } else {
                        *(unsigned short *)cmd->data = (unsigned short)data;
                    }
                } else {
                    size_t remaining = cmd->byte_count;
                    while (remaining != 0) {
                        unsigned char *buffer;
                        long data;
                        size_t count = remaining;
                        size_t offset = (cmd->byte_count - remaining);

                        if (count > 1) {
                            count = 1;
                        }

                        buffer = cmd->data + offset;

                        data = i2c_smbus_read_byte_data(
                                fd,
                                cmd->register_address + offset);

                        if (data < 0) {
                            rc = errno;
                            final_rc = rc;
                            break;
                        }

                        *buffer = (unsigned char)data;

                        remaining -= count;
                    }
                }
            }
        }

        flock(fd, LOCK_UN);
    }

    free(all_cmds);

    close(fd);

    return final_rc;
}
