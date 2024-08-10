#!/bin/bash

CWD=$(pwd)

export NODE_NUMBER=2
export LOG_DIR=$CWD/output

NOW=$(date +%Y.%m.%d_%Hh%Mm%Ss)
OUTPUTFILE=./out.$NOW.txt


touch $OUTPUTFILE; sbatch -N $NODE_NUMBER -o $OUTPUTFILE job.sbatch; tail -f $OUTPUTFILE