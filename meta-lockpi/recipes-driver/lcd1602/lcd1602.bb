DESCRIPTION = "LCD1602 Kernel Module from Git"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=da2da5d50f1177f23c37bf5701a5b07d"

inherit module

SRC_URI = "git://github.com/cu-ecen-aeld/final-project-anurag3301.git;branch=main;protocol=https"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git/lcd1602"

do_install:append() {
    install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
    install -m 0644 lcd1602.ko ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/

    install -d ${D}${sysconfdir}/modules-load.d
    echo "lcd1602" > ${D}${sysconfdir}/modules-load.d/lcd1602.conf
}

FILES:${PN} += "${sysconfdir}/modules-load.d/lcd1602.conf"
