# Since we are using only python3, let's provide /usr/bin/python as a symlink to python3
do_install_append() {
  install -d ${D}/${bindir}
  ln -s -r ${D}/${bindir}/python3 ${D}/${bindir}/python
}

FILES_${PN} += "${bindir}/python3 ${bindir}/python"
