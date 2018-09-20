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

binfiles = "xvcServer \
           "

pkgdir = "xvcServer"

do_install() {
  dst="${D}/usr/local/packages/${pkgdir}"
  bin="${D}/usr/local/bin"
  install -d $dst
  install -d $bin
  for f in ${binfiles}; do
    install -m 755 $f ${dst}/$f
    ln -snf ../packages/${pkgdir}/$f ${bin}/$f
  done
}

PACKAGEDIR = "${prefix}/local/packages"

FILES_${PN} = "${PACKAGEDIR}/xvcServer ${prefix}/local/bin"

# Inhibit complaints about .debug directories for the fand binary:

INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INHIBIT_PACKAGE_STRIP = "1"
