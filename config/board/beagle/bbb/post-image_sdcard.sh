#!/bin/sh
# post-image.sh for SD-CARD

BOARD_DIR="$(dirname $0)"
GENIMAGE_CFG="${BOARD_DIR}/genimage.cfg"

# copy the uEnv.txt to the output/images directory
cp ${BOARD_DIR}/uEnv_initrd.txt ${BINARIES_DIR}/uEnv.txt
cp ${BOARD_DIR}/zImage_initrd	${BINARIES_DIR}/zImage_initrd

GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"

rm -rf "${GENIMAGE_TMP}"

genimage \
    --rootpath "${TARGET_DIR}" \
    --tmppath "${GENIMAGE_TMP}" \
    --inputpath "${BINARIES_DIR}" \
    --outputpath "${BINARIES_DIR}" \
    --config "${GENIMAGE_CFG}"
