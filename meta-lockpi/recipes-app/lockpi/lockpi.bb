SUMMARY = "The lockpi app"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://../LICENSE;md5=da2da5d50f1177f23c37bf5701a5b07d"

SRC_URI = "git://github.com/cu-ecen-aeld/final-project-anurag3301.git;branch=main;protocol=https"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git/lockpi"

EXTRA_OEMAKE = 'CFLAGS="-Wall -Wextra -DFILEPATH=\\\"${sysconfdir}/lockpi/passkey.txt\\\""'

do_install() {
    install -d ${D}${bindir}
    install -m 0755 lockpi ${D}${bindir}/lockpi

    install -d ${D}${sysconfdir}/lockpi
    install -m 0644 passkey.txt ${D}${sysconfdir}/lockpi/passkey.txt
}

FILES:${PN} += "${bindir}/lockpi ${sysconfdir}/lockpi/passkey.txt"
