# This is a uboot extra customisation for our verison,
# we need the custom platform init

DEPENDS += "platform-init"

FILESEXTRAPATHS_prepend := "${THISDIR}/u-boot-xlnx:"

XILINX_EXTENSION = "-xilinx-iceotope"

UBOOT_ENV_track2 = "uEnv"

SRC_URI_append_track2 = " file://uEnv.txt"

## Adds our board config into the mix
SRC_URI_append = " file://0001-Adding-in-the-iceotop-bmc-config.patch \
    "
