This README file contains information on the contents of the assignment layer for Operating systems for embedded systems.

This layer has been develeped on Raspberry Pi.

Please see the corresponding sections below for details.


# How adding the assignment layer to your build

To add the assignment layer to build first clone this repository in poky directory:

```
$ cd ~/poky
$ git clone https://github.com/nicoladilillo/meta-assignment.git
```

After run the following command:
```
$ source oe-init-build-env  <your_target>
```

Then add this layer to build:
```
$ bitbake-layers add-layer ../meta-assignment
```

Next in ``recipies-heartrate/PPG/ppg.bb`` set the value of line 7 with your target, that by default is set to ``"raspberry"``
```
COMPATIBLE_MACHINE = "<your_target>"
```

Add the following line at the end of ``conf/local.conf``, in your target folder:
```
IMAGE_INSTALL_append = " ppg heartrate"
KERNEL_MODULE_AUTOLOAD += " ppg"
```

At the end build the new image:
```
$ bitbake core-image-minimal
```

# How run application
First run the machine (or turn on your device):
```
$ runqemu <your_target>
```

To execute the application run the command:
```
$ heartrate
```

## Note

To see the time to elaborate each single value uncomment the line 47 of ``../assignment/recipes-example/heartrate/files/heartrate.c`` and build the new image
```
#define DEBUG
```

# How configure on Raspberry Pi

After executing all previous instraction with <your_target> set to "raspberry" (any version will work) configure the interfaces. Move to:
```
$ cd ~/poky/meta/recipes-core/init-ifupdown/init-ifupdown-1.0/raspberrypi
```

In this place put a file named ``interfaces`` with the following content:
```
# /etc/network/interfaces -- configuration file for ifup(8), ifdown(8)
# The loopback interface auto lo
auto lo
iface lo inet loopback

# Wired or wireless interfaces auto eth0
auto eth0
iface eth0 inet static
	address 192.168.1.2
	netmask 255.255.255.0
	gateway 192.168.1.1
	network 192.168.1.0
```

And add the following line at the end of ``conf/local.conf``:
```
EXTRA_IMAGE_FEATURES ?= "debug-tweaks tools-debug eclipse-debug ssh-server-openssh"
IMAGE_FSTYPES = "tar.xz ext3 rpi-sdimg"
```

Then build the system as follows:
```
$ bitbake core-image-full-cmdline
```

Once the build process is completed, you will have a brand new Linux image ready for being copied to a Micro SD as
follows (assuming the Micro SD is available as /dev/sdb ; if a native Linux machine is used, the Micro SD is likely to
be available as / dev/mmcblk0 ).

Warning: double check which is the correct output device ( of=<output device> ) you are using. The following ommand can wipe out your hard disk.
```
$ sudo dd if=tmp/deploy/images/raspberrypi/core-image-full-cmdline-raspberrypi.rpi-sdimg of=/dev/sdb
```

Connect one side of yout ethernet cable to PC and the other to raspberry, put the Micro SD inside it and turn on the device. Set, on the ethernet interface of PC, a IP adress that belong to the same network of raspberry, for example 192.168.1.1. In this way we can connect to raspberry with the following command:
```
$ ssh root@192.168.1.2
```

Now is possibile execute all possible operation on target from PC, for istance run the application ``heartbreak``.

## Note

All this operations on raspberry can be done with different version (example: raspberry2, raspberry3, ...).

Is possible connect to raspberry also by UART connection just adding to ``conf/local.conf`` the following line:
```
ENABLE_UART = "1"
```