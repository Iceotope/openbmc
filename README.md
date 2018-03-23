# OpenBMC

OpenBMC is an open software framework to build a complete Linux image for a Board Management Controller (BMC).

OpenBMC uses the [Yocto Project](https://www.yoctoproject.org) as the underlying building and distro generation framework.

## Exanest Fork

Iceotope have forked the orignal [OpenBMC](http://www.github/facebook/openbmc) and begun porting the code base
to run on the Zynq and UltraZynq based controllers for the Track1 and Track2 mezzanine cards.
## Contents

This repository includes 3 set of layers:

* **OpenBMC Common Layer** - Common packages and recipes can be used in different types of BMC.
* **BMC System-on-Chip (SoC) Layer** - SoC specific drivers and tools. This layer includes the bootloader (u-boot) and the Linux kernel. Both the bootloader and Linux kernel shall include the hardware drivers specific for the SoC.
* **Board Specific Layer** - Board specific drivers, configurations, and tools. This layer defines how to configure the image. It also defines what packages to be installed for an OpenBMC image for this board. Any board specific initialization and tools are also included in this layer.

## File structure

The Yocto naming pattern is used in this repository. A "`meta-layer`" is used to name a layer or a
category of layers. And `recipe-abc` is used to name a recipe. 

The project high level Yocto project is placed in a subdirectory, `yocto` and is pulled in from
their remote repos as described below.

The recipes for OpenBMC common layer are found in `common`.

The BMC SoC layer and board specific layer are grouped together based on the vendor/manufacturer name.
For example, all Facebook boards specific code should be in `meta-facebook`. While new Exanest code
is in a meta layer called `meta-exanest`. `meta-aspeed` includes source code for Aspeed SoCs, used by the
existing facebook boards.

Exanest boards will use the meta-xilinx, which is pulled in from a remote repo.

## How to build

Note: In the instruction set below, references to <platform> for some of the steps is an example only and need to be replaced with the respective platform when setting up for a different platform.

1. Set up the build environment based on the Yocto Project's [Quick Start Guide](http://www.yoctoproject.org/docs/current/yocto-project-qs/yocto-project-qs.html).

2. Clone the OpenBMC repository and other open source repositories:
 ```bash
 $ git clone -b iceotope-helium git@exanest-git.ics.forth.gr:Marc.Kelly/exanest-openbmc.git
 $ cd exanest-openbmc
 $ ./sync_yocto.sh
 $ ./sync_yocto_xilinx.sh
 ```

3. Initialize a build directory for the platform to build (e.g. track2). In the `exanest-openbmc` directory:
 ```bash
 $ source openbmc-init-build-env meta-exanest/meta-track2
 ```
 Choose between `meta-track1` and `meta-track2`, or any of the other platforms listed in the meta-facebook directory.
 After this step, you will be dropped into a build directory, `openbmc/build`.

4. Start the build within the build directory:
 In general to build for the platform:
 ```bash
 $ bitbake <platform>-image
 ```
 The build process automatically fetches all necessary packages and builds the complete image. The final build results are in `openbmc/build/tmp/deploy/images/<platform>`. The root password will be `0penBmc`, you may change this in the local configuration.

## Build Artifacts

In a change from the orignal facebook build articles the following and are found
in the `build/tmp/deploy/images/<platform>` firectory. 

* **boot.bin** - SPL image, in bootable format
* **u-boot-<platform\>.img** - u-boot image, in bootable format (mkimage output)
* **zImage-<platform\>.bin** - This the Linux kernel for the board. (__Not__ wrapped in uboot-image header)
* **<platform\>-image-<platform\>.cpio.gzt** - This is the rootfs for the board
* **flash-<platform\>** - This is the boot flash image. Contains SPL and u-boot. This goes in the QSPI.
* **fit-<platform\>.itb** -  Complete Linux image, FIT formated. Has Kernel, Device Tree, RootFS and FPGA Image.


To install on hardware, use the Xilinx tool to flash **flash-<platform\>** into the QSPI. That will
boot the board all the way to u-boot, then put **fit-<platform\>.itb** on the SD (or tftp server) and
continue the boot.

By setting the `bootcmd` env in u-boot you can have things automatically boot.

`run fitboot` will pull the image from a DHCP assigned TFTP server and boot into linux.

## Contact

Marc Kelly <Marc.Kelly@iceotope.com>
