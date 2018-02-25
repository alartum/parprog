#!/bin/bash

JOB=$1
TIME_FILE=$2
NUM=${JOB%%.*}
OUT_NAME="timing.o$NUM"
ERR_NAME="timing.e$NUM"

while [ ! -f $ERR_NAME ]
do 
	sleep 1
done

if [ ! -s $ERR_NAME ]
then
	rm $ERR_NAME
fi

python avg.py $OUT_NAME > $TIME_FILE
rm $OUT_NAME

rm "$TIME_FILE.wait"
