#!/bin/bash

#  change this if you wish [1 - delete, 0 - no delete]
DELETE_INTS=0

HERE=$(dirname $(readlink -f $0))
INPS="$HERE/inps"
INTS="$HERE/ints"
LOGS="$HERE/logs"
PLOTS="$HERE/plots"
SETTINGS_FILE="settings.inp"

rm $HERE/res*
cp $INPS/$SETTINGS_FILE $HERE
rm -r $INPS $LOGS $PLOTS

if [ "$DELETE_INTS" -eq 1 ]; then
    echo " Deleting integrals."
    rm -r $INTS
fi