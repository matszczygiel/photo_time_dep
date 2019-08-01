#use with job name as a argument

#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo " Illegal number of parameters, usage: ./run.sh <job name>"
    exit 1
fi

XGTOPW_PATH="/home/mateusz/workspace/gtopw/build/Release/"
PHOTO_TD_PATH="/home/mateusz/workspace/photo_time_dep/build/Release/"

#  change this if you wish [1 - spherical, 0 - cartesian]
REPRESENTATION=1


HERE=$(dirname $(readlink -f $0))
mkdir ints inps logs
INTS="$HERE/ints"
INPS="$HERE/inps"
LOGS="$HERE/logs"
SETTINGS_FILE="settings.inp"
JOB_NAME=$1

sed -i -e "s/^JOB_NAME .*/JOB_NAME                        ${JOB_NAME}/g" $SETTINGS_FILE
sed -i -e "s|^RESOURCES_PATH .*|RESOURCES_PATH                        ${INTS}|g" $SETTINGS_FILE
sed -i -e "s|^OUT_PATH .*|OUT_PATH                        ${HERE}|g" $SETTINGS_FILE

if [ "$REPRESENTATION" -eq 1 ]; then
    echo " Using spherical representation."
    sed -i -e "s|^FILE_1E .*|FILE_1E                        file1E_${JOB_NAME}_sph.F|g" $SETTINGS_FILE
    sed -i -e "s|^REPRESENTATION .*|REPRESENTATION                        spherical|g" $SETTINGS_FILE
elif [ "$REPRESENTATION" -eq 0 ]; then
    echo " Using cartesian representation."
    sed -i -e "s|^FILE_1E .*|FILE_1E                        file1E_${JOB_NAME}_crt.F|g" $SETTINGS_FILE
    sed -i -e "s|^REPRESENTATION .*|REPRESENTATION                        cartesian|g" $SETTINGS_FILE
else
    echo " Unkown representation."
    exit 1
fi

mv $SETTINGS_FILE $INPS

cd $INPS
$PHOTO_TD_PATH./main $SETTINGS_FILE -prep >$LOGS/log_prep.out

if [ "$?" -ne 0 ]; then
    echo " Preparation of xgtopw input failed."
    exit 1
fi

sed -i "7 a \$PATH\n$INTS/\n\$END" $JOB_NAME.inp
$XGTOPW_PATH./xgtopw $JOB_NAME.inp >$LOGS/log_xgtopw.out

if [ "$?" -ne 0 ]; then
    echo " Integrals computation (xgtopw) failed."
    exit 1
fi


#length gauge
sed -i -e "s|^GAUGE .*|GAUGE                        length|g" $SETTINGS_FILE
sed -i -e "s|^OUT_FILE .*|OUT_FILE                        res_len.out|g" $SETTINGS_FILE

$PHOTO_TD_PATH./main $SETTINGS_FILE >$LOGS/log_td_len.out

if [ "$?" -ne 0 ]; then
    echo " Length gauge computation (photo_td) failed."
    exit 1
fi


#velocity gauge
sed -i -e "s|^GAUGE .*|GAUGE                        velocity|g" $SETTINGS_FILE
sed -i -e "s|^OUT_FILE .*|OUT_FILE                        res_vel.out|g" $SETTINGS_FILE

$PHOTO_TD_PATH./main $SETTINGS_FILE >$LOGS/log_td_vel.out

if [ "$?" -ne 0 ]; then
    echo " Velocity gauge computation (photo_td) failed."
    exit 1
fi


cd $HERE
rm -r $INTS

./plot.py $JOB_NAME