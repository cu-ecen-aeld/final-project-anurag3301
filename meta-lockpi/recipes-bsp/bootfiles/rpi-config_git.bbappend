SUMMARY = "Enable i2c"
LICENSE = "MIT"
do_deploy:append:raspberrypi4-64() {
    CONFIG=${DEPLOYDIR}/${BOOTFILES_DIR_NAME}/config.txt

    echo "dtparam=i2c_arm=on" >>$CONFIG
}
