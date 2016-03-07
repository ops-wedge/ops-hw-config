# ops-hw-config

## What is ops-hw-config?
The ops-hw-config repository contains all of the hardware description files for all of the platforms supported by OpenSwitch. The hardware description files contain the information to enumerate what hardware capabilites are supported. These files are written in YAML. The repository also contains an extended YAML parser library that other daemons can use to read this information.

What is the structure of the repository?
----------------------------------------
Each vendor has a separate directory which contains a subdirectory for each supported platform. Each platform directory contains the hardware description files for that platform.

What is the license?
--------------------
The ops-hw-config files may be used under the terms of the Apache 2.0 license. For more details refer to [COPYING](COPYING).

What other documents are available?
===================================
For a high level design of ops-hw-config, refer to [DESIGN.md](DESIGN.md).
For answers to common questions, read [FAQ.md](FAQ.md).
For the current list of contributors and maintainers, refer to [AUTHORS.md](AUTHORS.md).

For general information about OpenSwitch project refer to [http://openswitch.net](http://openswitch.net).
