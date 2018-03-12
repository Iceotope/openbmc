SUMMARY = "Facebook OpenBMC base packages"

LICENSE = "GPLv2"
PR = "r1"

inherit packagegroup

RDEPENDS_${PN} += " \
  i2c-tools \
  kernel-modules \
  lmsensors-sensors \
  logrotate \
  os-release \
  passwd-util \
  rsyslog \
  tzdata \
  u-boot-xlnx \
  u-boot-fw-utils \
  "
