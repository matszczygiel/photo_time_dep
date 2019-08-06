#!/bin/bash

HERE=$(dirname $(readlink -f $0))
INPS="$HERE/inps"
INTS="$HERE/ints"
LOGS="$HERE/logs"
SETTINGS_FILE="settings.inp"

rm $HERE/res*
rm $HERE/norm*
cp $INPS/$SETTINGS_FILE $HERE
rm -r $INPS $LOGS $INTS