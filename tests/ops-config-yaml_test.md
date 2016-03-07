# ops-config-yaml Test Cases

[TOC]

##  add subsystem ##
### Objective ###
Verify that subsystems are added (or not) as appropriate.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
  [s1]
```
### Description ###
1. Try to add invalid subsystem
 - Verify that the add fails
2. Try to add a valid subsystem
 - Verify that it passes
3. Try to add the same subsystem again
 - Verify that it fails
4. Try to add a second valid subsystem
 - Verify that it passes

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  parse devices ##
### Objective ###
Verify that devices.yaml file parsing succeeds or fails for good/invalid devices.yaml file.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
  [s1]
```
### Description ###
1. Try to parse invalid devices.yaml file
 - Verify that it fails
2. Try to parse a valid devices.yaml file
 - Verify that it passes

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  find file ##
### Objective ###
Verify that find a file (devices.yaml) passes for good manifest file entry.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
  [s1]
```
### Description ###
1. Try to find the devices.yaml file with a valid manifest.yaml file entry
 - Verify that it passes

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  init devices ##
### Objective ###
Verify that the yaml_init_devices API calls the correct number of op commands. No i2c calls are made.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
  [s1]
```
### Description ###
1. Call yaml_init_devices()
 - Verify that exactly 3 op commands are used.

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  parse good files ##
### Objective ###
Verify that all yaml files are correctly parsed when the files are valid.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
  [s1]
```
### Description ###
1. Attempt to parse devices.yaml
 - Verify that it succeeds
2. Attempt to parse thermal.yaml
 - Verify that it succeeds
3. Attempt to parse ports.yaml
 - Verify that it succeeds
4. Attempt to parse fans.yaml
 - Verify that it succeeds
5. Attempt to parse power.yaml
 - Verify that it succeeds
6. Attempt to parse leds.yaml
 - Verify that it succeeds

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  parse bad files ##
### Objective ###
Verify that all yaml files are not parsed when the files are invalid.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
  [s1]
```
### Description ###
1. Attempt to parse devices.yaml
 - Verify that it fails
2. Attempt to parse thermal.yaml
 - Verify that it fails
3. Attempt to parse ports.yaml
 - Verify that it fails
4. Attempt to parse fans.yaml
 - Verify that it fails
5. Attempt to parse power.yaml
 - Verify that it fails
6. Attempt to parse leds.yaml
 - Verify that it fails

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.

##  parse unspecified bad files ##
### Objective ###
Verify that calls to parse yaml files succeeds when there is not a file for that hardware type. The call should succeed but will return zero elements.
### Requirements ###
 - Virtual Mininet Test Setup

### Setup ###
#### Topology Diagram ####
```
  [s1]
```
### Description ###
1. Attempt to parse devices.yaml
 - Verify that it succeeds
2. Attempt to parse thermal.yaml
 - Verify that it succeeds
3. Attempt to parse ports.yaml
 - Verify that it succeeds
4. Attempt to parse fans.yaml
 - Verify that it succeeds
5. Attempt to parse power.yaml
 - Verify that it succeeds
6. Attempt to parse leds.yaml
 - Verify that it succeeds

### Test Result Criteria ###
#### Test Pass Criteria ####
All verifications pass.
#### Test Fail Criteria ####
One or more verifications fail.
