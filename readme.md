
# JerryScript for Zephyr, JerryZ

The JerryZ is a Zephyr compatible JavaScript engine, made from a modified JerryScript build process. The JerryZ repo can be found in this [repo](https://github.com/moduhub/zephyr_jerryscript). 

This repository contains tests of the JerryScript in Zephyr, AKA JerryZ, follow the instruction for installation.

# Installation
On an empty folder, run the command:
```
west init -m https://github.com/ig-66/jerry_z_tests
```
This command will result in the following:
```
[folder_where_the_command_was_executed]
    + .west 
    + jerry_z_tests
        + west.yml
        + ...
``` 
Now run:
```
west update
```
# Running the Examples
## Selecting the Example to Build
To select the example simply run the `menuconfig` or `guiconfig`:
```
west build -b <board> -t guiconfig
```
## Flashing the Build
Simply run:
```
west flash
```

## Snapshot Load Example
In this example, the application will load and execute a snapshot file that is present in the mass storage.

The snapshot file, named `.snapshot`, must be placed in the selected storage device before the program runs, it can be placed in the storage through the USB. 

The snapshot file can be created using a JerryScript on Linux or Windows. To learn more visit the [official JerryScript repo](https://github.com/jerryscript-project/jerryscript).

To select the snapshot load and execution example, open Zephyr's `guiconfig` or `menuconfig` and select "Build the snapshot file sample" choice under the "JerryZ sample to be built" menu.

When it is selected, the configuration options to make the device: use the SDHC over SPI; put it USB mass storage; use long file names; and name mount point to SD, are automatically selected.