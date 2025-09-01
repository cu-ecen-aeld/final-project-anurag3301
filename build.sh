#!/bin/bash
set -e

git submodule init
git submodule sync
git submodule update

source poky/oe-init-build-env

LAYERS_DIR=$(pwd)/..

if ! grep -q "meta-raspberrypi" conf/bblayers.conf; then
    bitbake-layers add-layer "$LAYERS_DIR/meta-raspberrypi"
fi

for layer in meta-oe meta-python meta-networking; do
    if ! grep -q "$layer" conf/bblayers.conf; then
        bitbake-layers add-layer "$LAYERS_DIR/meta-openembedded/$layer"
    fi
done

if grep -q '^MACHINE ?=' conf/local.conf; then
    sed -i 's/^MACHINE ?=.*/MACHINE ?= "raspberrypi4-64"/' conf/local.conf
elif grep -q '^MACHINE =' conf/local.conf; then
    sed -i 's/^MACHINE =.*/MACHINE ?= "raspberrypi4-64"/' conf/local.conf
else
    echo 'MACHINE ?= "raspberrypi4-64"' >> conf/local.conf
fi

echo "Layers added and MACHINE set to raspberrypi4-64"
echo " -------- Building core-image-minimal ----------"

bitbake core-image-minimal
