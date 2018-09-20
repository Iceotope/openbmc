# Xilinx Virtual Cable demo/example. Copied over from Xilinx
# projects into our build to drive the JTAG system.

DESCRIPTION = "Xilinx virtual cable daemon"
SECTION = "base"
DEPENDS = ""
LICENSE = "CC0-1.0"
ALLOW_EMPTY_${PN} = "1"
FILESEXTRAPATHS_append := "${THISDIR}/files:"

LIC_FILES_CHKSUM = "file://LICENSE;md5=7bae63a234e80ee7c6427dce9fdba6cc"


SRC_URI = "file://LICENSE \
           file://Makefile \
           file://xvcServer.c \
          "
S = "${WORKDIR}"

binfiles = "xvcServer"

pkgdir = "xvcServer"
