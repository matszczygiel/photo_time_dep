#use with job name as a argument

#!/bin/bash

XGTOPW_PATH="/home/mateusz/workspace/gtopw/build/Release/"
PHOTO_TD_PATH="/home/mateusz/workspace/photo_time_dep/build/Release/"

HERE=$(dirname $(readlink -f $0))
mkdir ints inps logs
INTS="$HERE/ints"
INPS="$HERE/inps"
LOGS="$HERE/logs"
SETTINGS_FILE="settings.inp"
JOB_NAME=$1

sed -i -e "s/^JOB_NAME .*/JOB_NAME                        ${JOB_NAME}/g" $SETTINGS_FILE
sed -i -e "s|^RESOURCES_PATH .*|RESOURCES_PATH                        ${INTS}|g" $SETTINGS_FILE
sed -i -e "s|^FILE_1E .*|FILE_1E                        file1E_${JOB_NAME}.F|g" $SETTINGS_FILE
sed -i -e "s|^OUT_PATH .*|OUT_PATH                        ${HERE}|g" $SETTINGS_FILE
mv $SETTINGS_FILE $INPS

cd $INPS
$PHOTO_TD_PATH./main $SETTINGS_FILE -prep >$LOGS/log_prep.out

sed -i "6 a \$PATH\n$INTS/\n\$END" $JOB_NAME.inp
$XGTOPW_PATH./xgtopw $JOB_NAME.inp >$LOGS/log_xgtopw.out

$PHOTO_TD_PATH./main $SETTINGS_FILE >$LOGS/log_photo_td.out

cd $HERE