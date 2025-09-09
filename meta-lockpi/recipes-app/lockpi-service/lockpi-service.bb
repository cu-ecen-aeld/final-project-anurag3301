SUMMARY = "Startup service for lockpi application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://lockpi.init;md5=0a5f57f309b9814d04cac6c989796764"

SRC_URI = "file://lockpi.init"

S = "${WORKDIR}"

inherit update-rc.d

INITSCRIPT_NAME = "lockpi"
INITSCRIPT_PARAMS = "defaults 99"

do_install() {
    install -d ${D}${sysconfdir}/init.d
    install -m 0755 ${WORKDIR}/lockpi.init ${D}${sysconfdir}/init.d/lockpi
}

FILES:${PN} += "${sysconfdir}/init.d/lockpi"
