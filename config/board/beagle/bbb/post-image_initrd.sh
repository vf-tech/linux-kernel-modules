#!/bin/sh
# post-image.sh for INITRD

BOARD_DIR="$(dirname $0)"
# copy the zImage to the BOARD directory
cp ${BINARIES_DIR}/zImage	${BOARD_DIR}/zImage_initrd
