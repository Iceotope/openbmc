inherit exanest_kernel_fitimage
inherit extrausers
#inherit image_types_uboot

# Base this image on core-image-minimal
include recipes-core/images/core-image-minimal.bb

## MPK removed these they seem facebook specific
# Changing the image compression from gz to lzma achieves 30% saving (~3M).
# However, the current u-boot does not have lzma enabled. Stick to gz
# until we generate a new u-boot image.
#IMAGE_FSTYPES += "cpio.lzma.u-boot"

UBOOT_IMAGE_LOADADDRESS ?= "0x00080000"
UBOOT_IMAGE_ENTRYPOINT ?= "0x00080000"
UBOOT_RAMDISK_LOADADDRESS ?= "0x00800000"

SPL_BINARYNAME = "boot.bin"
DEVICE_TREE_BINARY_NAME = "track1-zynq7.dtb"

## FB orignal list from one of their servers, we cut it down for now
## Include modules in rootfs
##IMAGE_INSTALL += " \
#  packagegroup-openbmc-base \ *
#  healthd \ *
#  fbutils \
#  fan-util \ -
#  fscd \ -
#  watchdog-ctrl \ *
#  sensor-setup \ -
#  ipmid \
#  packagegroup-openbmc-net \ *
#  packagegroup-openbmc-python3 \ *
#  packagegroup-openbmc-rest3 \ *
#  fruid \ *
#  ipmbd \ *
#  bic-cached \ -
#  bic-util \ -
#  sensor-util \ *
#  sensor-mon \ *
#  gpiod \ -
#  front-paneld \ -
#  power-util \ *
#  mterm\ *
#  cfg-util \ *
#  fw-util \ -
#  fpc-util \ -
#  me-util \ -
#  log-util \ *
#  lldp-util \ *
#  spatula \ *
#  openbmc-utils \ *
#  conf-util \ -
#  ncsi-util \ -
#  "

IMAGE_INSTALL += " \
  packagegroup-openbmc-base \
  packagegroup-openbmc-net \
  packagegroup-openbmc-python3 \
  packagegroup-openbmc-rest3 \
  watchdog-ctrl \
  lldp-util \
  bitbang \
  flashrom \
  power-util \
  log-util \
  openbmc-utils \
  fpga-image \
  mterm \
  fruid \
  sensor-util \
  sensor-mon \
  ipmbd \
  ipmb-util \
  healthd \
  cfg-util \
  ipmi-util \
  ipmitool \
  vboot-utils \
  spatula \
  track1-sensors \
  nano \
  xvc \
  retimers \
  "

IMAGE_INSTALL_append = " bind-utils"

IMAGE_FEATURES += " \
  ssh-server-openssh \
  tools-debug \
  "

DISTRO_FEATURES += " \
  ext2 \
  nfs \
  usbgadget \
  usbhost \
  "
# Was removed for now
#  ipv6

