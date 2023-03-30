#!/bin/sh

BOARD_DIR="$(dirname $0)"
SDKNAME="$2"
SDK_COMMAND="tar cf $BINARIES_DIR/$SDKNAME.tar -C $HOST_DIR ."

# tar SDK to output/images
echo "SYSTEM VARIABLES:"
echo $BOARD_DIR
echo $BINARIES_DIR
echo $HOST_DIR
echo $BINARIES_DIR/$SDKNAME.tar
echo $SDKNAME
echo $SDK_COMMAND

echo "\nRemoving current-sdk archive..."
rm $BINARIES_DIR/$SDKNAME.tar

echo "\nCreating new sdk archive..."
$SDK_COMMAND

exit $?
