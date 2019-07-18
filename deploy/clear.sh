#!/bin/bash

HERE=$(dirname $(readlink -f $0))
INPS="$HERE/inps"
LOGS="$HERE/logs"
SETTINGS_FILE="settings.inp"

rm $HERE/res*
cp $INPS/$SETTINGS_FILE $HERE
rm -r $INPS $LOGS