#!/usr/bin/python
#
# (c) Copyright 2015 Hewlett Packard Enterprise Development LP
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.
#

# OPS_TODO: Need to make cfg_yaml_ut be available from cached build.
# Note: This test will only run if a build is done for this repo in devenv prior to running make devenv_ct_test.

import os
import sys
import time
import subprocess
import pytest

from opsvsi.docker import *
from opsvsi.opsvsitest import *

SRC_PATH = "./src/ops-config-yaml/"
TEST_PRG_PATH = SRC_PATH + "build/tests/cfg_yaml_ut"
TEST_FILES_PATH = SRC_PATH + "tests/yaml_files"

# Test case configuration.

def get_ip(s):
    out = s.cmd("ifconfig eth0 | grep inet")
    ip = out.split()[1].split(':')[1]
    return ip

def cfgy_tests_passed(out):
    num_tests = 0
    passed_tests = 0

    lines = out.splitlines()

    for line in lines:
        # Looking for "[==========] Running xxx test"
        if "[==========] Running " in line:
            num_tests = int(line.split()[2])
    if num_tests <= 0:
        return False
    for line in lines:
        # Looking for "[  PASSED  ] xxx"
        if "[  PASSED  ]" in line:
            passed_tests = int(line.split()[3])
    if passed_tests == num_tests:
        return True
    return False

# Create a topology with only one switch
class mySingleSwitchTopo( Topo ):
    """Single switch, no hosts
    """

    def build(self, hsts=0, sws=1, n_links=0, **_opts):
        self.hsts = hsts
        self.sws = sws

        "Add the switches to the topology."
        for s in irange(1, sws):
            switch = self.addSwitch('s%s' %s)

class configyamlTest(OpsVsiTest):

    def setupNet(self):

        # Create a topology with one OpenSwitch switch
        host_opts = self.getHostOpts()
        switch_opts = self.getSwitchOpts()
        configyaml_topo = mySingleSwitchTopo(sws=1, hopts=host_opts, sopts=switch_opts)

        self.net = Mininet(configyaml_topo, switch=VsiOpenSwitch,
                           host=Host, link=OpsVsiLink,
                           controller=None, build=True)

    # Pre-test setup
    def test_pre_setup(self):
        s1 = self.net.switches[0]

        ip = get_ip(s1)

        info("ip is [%s]" % ip)

        s1.cmd("mkdir /tmp/yaml_files")
        os.system("scp -oStrictHostKeyChecking=no " + TEST_PRG_PATH + " root@" + ip + ":/tmp")
        os.system("scp -r -oStrictHostKeyChecking=no " + TEST_FILES_PATH + " root@" + ip + ":/tmp")

        if s1.cmd("ls /tmp/cfg_yaml_ut").split()[0] != "/tmp/cfg_yaml_ut":
            info("Need to build the test program")
            return False
        return True

    # Post-test cleanup
    def test_post_cleanup(self):
        s1 = self.net.switches[0]
        s1.cmd("rm -rf /tmp/yaml_files /tmp/cfg_yaml_ut")

    def test_001_config_yaml(self):

        info("\n============= test_001_config_yaml =============\n")

        # Copy the test program and test files to the switch
        if not self.test_pre_setup():
            return

        s1 = self.net.switches[0]

        # Run the tests on the switch
        out = s1.cmd("/tmp/cfg_yaml_ut")

        assert cfgy_tests_passed(out) == True, "Tests failed, test output is...\n\n%s" % out

        # Cleanup
        self.test_post_cleanup()

class Test_configyaml:

    def setup(self):
        pass

    def teardown(self):
        pass

    def setup_class(cls):
        # Create the Mininet topology based on Mininet.
        Test_configyaml.test = configyamlTest()

    def teardown_class(cls):

        # Stop the Docker containers and mininet topology
        Test_configyaml.test.net.stop()

    def setup_method(self, method):
        pass

    def teardown_method(self, method):
        pass

    def __del__(self):
        del self.test

    def test_configyaml(self):
        self.test.test_001_config_yaml()
