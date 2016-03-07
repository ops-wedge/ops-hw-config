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

#include <gtest/gtest.h>

#include "../include/config-yaml.h"
#include "../src/config-yaml.cpp"

#include "cfg_yaml_ut.h"

#define GOOD_MANIFEST "good.manifest.yaml"
#define BAD_MANIFEST "bad.manifest.yaml"
#define EMPTY_MANIFEST "empty.manifest.yaml"
#define MANIFEST_FILE "manifest.yaml"

int ops_cnt;

int
unlink_file(const char *dir, const char *filename)
{
    char cmd[2048];

    sprintf(cmd,"/bin/rm -f %s/%s", dir, filename);

    return (system(cmd));
}

int
link_file(const char *dir, const char *filename, const char *linkname)
{
    char cmd[2048];

    sprintf(cmd,"/bin/ln -s %s/%s %s/%s", dir, filename, dir, linkname);

    return (system(cmd));
}

/* Define Test Suite class for customer setup and teardown functions. */
class CfgYamlTestSuite : public testing::Test
{
    public:
        YamlConfigHandle cy_handle;

    CfgYamlTestSuite() {
        printf("Test constructor.\n");
    }

    virtual ~CfgYamlTestSuite() {
        printf("Test destructor.\n");
    }

    void SetUp(void) {
        printf("Test setup.\n");

        /* Create a new CFG_YAML handle. */
        cy_handle = yaml_new_config_handle();
        ASSERT_NE(cy_handle, (YamlConfigHandle) NULL);

        printf("Test setup completed.\n");
    }

    void TearDown(void) {

        printf("Test tear down\n");

        /* Currently we don't have mechanism to free the memory
         * allocated in the CFG_YAML library.
         * The memory is for sure leaked during the test run. */

        printf("Test tear down completed.\n");
    }
};

/* Test Fixture. */
/*************************************************************//**
 * Verify that the yaml_add_subsystem API
 * - fails with an invalid hwdesc directory pointer
 * - works with a good manifest file
 * - won't let you add the same subsystem twice
 * - allows multiple subsystems to be added
 ****************************************************************/
TEST_F(CfgYamlTestSuite, cfg_001_yaml_add_subsystem) {

    char    cwd[1024];
    int     rc = 0;

    /* Test YAML files are stored in ./yaml_files dir. */
    rc = snprintf(cwd, sizeof(cwd), "%s/%s", CFG_YAML_UT_FILE_DIR, "yaml_files");
    ASSERT_GT(rc, 0);

    /* Try creating a subsystem with invalid directory.
     * The look up for manifest file should fail. */
    printf("Create a new base SUBSYSTEM with invalid manifest directory.\n");
    rc = yaml_add_subsystem(cy_handle, "test_subsys", "junk_dir");
    ASSERT_NE(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
    link_file(cwd, GOOD_MANIFEST, MANIFEST_FILE);

    /* Try creating a subsystem. It should PASS. */
    printf("Create a new base SUBSYSTEM with valid params.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_EQ(rc, 0);

    /* Try re-creating a subsystem. It should FAIL. */
    printf("Re-create base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_NE(rc, 0);

    /* Try creating another subsystem. It should PASS. */
    printf("Create a new test-base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, "test-base", cwd);
    ASSERT_EQ(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
}

/* Test Fixture. */
/*************************************************************//**
 * Verify that the yaml_parse_devices API
 * - fails with an invalid subsystem
 * - works with a good devices file
 ****************************************************************/
TEST_F(CfgYamlTestSuite, cfg_002_yaml_parse_devices) {
    char    cwd[1024];
    int     rc = 0;

    /* Test YAML files are stored in ./yaml_files dir. */
    rc = snprintf(cwd, sizeof(cwd), "%s/%s", CFG_YAML_UT_FILE_DIR, "yaml_files");
    ASSERT_GT(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
    link_file(cwd, GOOD_MANIFEST, MANIFEST_FILE);

    /* Try creating a subsystem. It should PASS. */
    printf("Create a new base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_devices() with a uninitialized subsystem.
     * The call should fail */
    printf("Parse devices with invalid subsystem.\n");
    rc = yaml_parse_devices(cy_handle, "junk_subsystem");
    ASSERT_NE(rc, 0);

    /* Call yaml_parse_devices() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse devices with valid subsystem.\n");
    rc = yaml_parse_devices(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
}

/* Test Fixture. */
/*************************************************************//**
 * Verify that the yaml_find_file API
 * - finds the expected values from a good manifest file
 ****************************************************************/
TEST_F(CfgYamlTestSuite, cfg_003_yaml_find_file) {
    char    cwd[1024];
    int     rc = 0;
    const YamlFile    *yfp = NULL;

    /* Test YAML files are stored in ./yaml_files dir. */
    rc = snprintf(cwd, sizeof(cwd), "%s/%s", CFG_YAML_UT_FILE_DIR, "yaml_files");
    ASSERT_GT(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
    link_file(cwd, GOOD_MANIFEST, MANIFEST_FILE);

    /* Try creating a subsystem. It should PASS. */
    printf("Create a new base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_devices() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse devices with valid subsystem.\n");
    rc = yaml_parse_devices(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_find_file() to get info from manifest file */
    printf("Get info for devices from manifest file.\n");
    yfp = yaml_find_file(cy_handle, BASE_SUBSYSTEM, "devices");
    ASSERT_EQ(strcmp(yfp->name,"devices"), 0);
    ASSERT_EQ(strcmp(yfp->filename,"devices.yaml"), 0);

    unlink_file(cwd, MANIFEST_FILE);
}

/* Test Fixture. */
/*************************************************************//**
 * Verify that the yaml_init_devices API
 * - calls i2c_execute with the correct # of op cmds
 * -   note: no actual i2c calls are made.
 ****************************************************************/
TEST_F(CfgYamlTestSuite, cfg_004_yaml_init_devices) {
    char    cwd[1024];
    int     rc = 0;

    /* Test YAML files are stored in ./yaml_files dir. */
    rc = snprintf(cwd, sizeof(cwd), "%s/%s", CFG_YAML_UT_FILE_DIR, "yaml_files");
    ASSERT_GT(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
    link_file(cwd, GOOD_MANIFEST, MANIFEST_FILE);

    /* Try creating a subsystem. It should PASS. */
    printf("Create a new base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_devices() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse devices with valid subsystem.\n");
    rc = yaml_parse_devices(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_init_devices() to fake the init of the devices */
    printf("Initialize the devices.\n");
    ops_cnt = 0;
    rc = yaml_init_devices(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);
    /* There should be 3 devices each with 1 command counted in ops_cnt */
    ASSERT_EQ(ops_cnt, 3);

    ops_cnt = 0;
    unlink_file(cwd, MANIFEST_FILE);
}

/* Test Fixture. */
/*************************************************************//**
 * Verify that the yaml_parse_* APIs
 * - correctly parses the various yaml files when they are good.
 ****************************************************************/
TEST_F(CfgYamlTestSuite, cfg_005_yaml_parse_good_files) {
    char    cwd[1024];
    int     rc = 0;

    /* Test YAML files are stored in ./yaml_files dir. */
    rc = snprintf(cwd, sizeof(cwd), "%s/%s", CFG_YAML_UT_FILE_DIR, "yaml_files");
    ASSERT_GT(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
    link_file(cwd, GOOD_MANIFEST, MANIFEST_FILE);

    /* Try creating a subsystem. It should PASS. */
    printf("Create a new base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_devices() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse devices with valid yaml.\n");
    rc = yaml_parse_devices(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_thermal() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse thermal with valid yaml.\n");
    rc = yaml_parse_thermal(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_ports() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse ports with valid yaml.\n");
    rc = yaml_parse_ports(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_fans() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse fans with valid yaml.\n");
    rc = yaml_parse_fans(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_psus() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse power with valid yaml.\n");
    rc = yaml_parse_psus(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_leds() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse leds with valid yaml.\n");
    rc = yaml_parse_leds(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
}

/* Test Fixture. */
/*************************************************************//**
 * Verify that the yaml_parse_* APIs
 * - fails when parsing bad yaml files.
 ****************************************************************/
TEST_F(CfgYamlTestSuite, cfg_006_yaml_parse_bad_files) {
    char    cwd[1024];
    int     rc = 0;

    /* Test YAML files are stored in ./yaml_files dir. */
    rc = snprintf(cwd, sizeof(cwd), "%s/%s", CFG_YAML_UT_FILE_DIR, "yaml_files");
    ASSERT_GT(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
    link_file(cwd, BAD_MANIFEST, MANIFEST_FILE);

    /* Try creating a subsystem. It should PASS. */
    printf("Create a new base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_devices() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse devices with invalid yaml.\n");
    rc = yaml_parse_devices(cy_handle, BASE_SUBSYSTEM);
    ASSERT_NE(rc, 0);

    /* Call yaml_parse_thermal() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse thermal with invalid yaml.\n");
    rc = yaml_parse_thermal(cy_handle, BASE_SUBSYSTEM);
    ASSERT_NE(rc, 0);

    /* Call yaml_parse_ports() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse ports with invalid yaml.\n");
    rc = yaml_parse_ports(cy_handle, BASE_SUBSYSTEM);
    ASSERT_NE(rc, 0);

    /* Call yaml_parse_fans() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse fans with invalid yaml.\n");
    rc = yaml_parse_fans(cy_handle, BASE_SUBSYSTEM);
    ASSERT_NE(rc, 0);

    /* Call yaml_parse_psus() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse power with invalid yaml.\n");
    rc = yaml_parse_psus(cy_handle, BASE_SUBSYSTEM);
    ASSERT_NE(rc, 0);

    /* Call yaml_parse_leds() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse leds with valid subsystem.\n");
    rc = yaml_parse_leds(cy_handle, BASE_SUBSYSTEM);
    ASSERT_NE(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
}

/* Test Fixture. */
/*************************************************************//**
 * Verify that the yaml_parse_* APIs
 * - succeeds when the manifest file doesn't point to yaml files.
 ****************************************************************/
TEST_F(CfgYamlTestSuite, cfg_007_yaml_parse_unspecified_bad_files) {
    char    cwd[1024];
    int     rc = 0;

    /* Test YAML files are stored in ./yaml_files dir. */
    rc = snprintf(cwd, sizeof(cwd), "%s/%s", CFG_YAML_UT_FILE_DIR, "yaml_files");
    ASSERT_GT(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
    link_file(cwd, EMPTY_MANIFEST, MANIFEST_FILE);

    /* Try creating a subsystem. It should PASS. */
    printf("Create a new base SUBSYSTEM.\n");
    rc = yaml_add_subsystem(cy_handle, BASE_SUBSYSTEM, cwd);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_devices() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse devices with valid subsystem.\n");
    rc = yaml_parse_devices(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_thermal() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse thermal with no thermal in manifest file.\n");
    rc = yaml_parse_thermal(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_ports() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse ports with no ports in manifest file.\n");
    rc = yaml_parse_ports(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_fans() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse fans with no fans in manifest file.\n");
    rc = yaml_parse_fans(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_psus() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse power with no power in manifest file.\n");
    rc = yaml_parse_psus(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    /* Call yaml_parse_leds() with valid subsystem,
     * and valid directory. The call should pass */
    printf("Parse leds with no leds in manifest file.\n");
    rc = yaml_parse_leds(cy_handle, BASE_SUBSYSTEM);
    ASSERT_EQ(rc, 0);

    unlink_file(cwd, MANIFEST_FILE);
}

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
