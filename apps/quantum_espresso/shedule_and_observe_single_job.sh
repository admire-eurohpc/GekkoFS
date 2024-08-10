#!/bin/bash

CWD=$(pwd)

export NODE_NUMBER=4
export LOG_DIR=$CWD/output

NOW=$(date +%Y.%m.%d_%Hh%Mm%Ss)
OUTPUTFILE=./out.$NOW.txt

touch $OUTPUTFILE
sbatch --exclude=cpu0397 -N $NODE_NUMBER -o $OUTPUTFILE job_NHR.sbatch
tail -f $OUTPUTFILE

