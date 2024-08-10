#!/bin/bash

CWD=$(pwd)

export PARAM_NUMNODES="${PARAM_NUMNODES:-8}"

export OUTPUT_DIR=$CWD/output
mkdir -p "${OUTPUT_DIR}"

NOW=$(date +%Y.%m.%d-%Hh%Mm%Ss)
OUTPUTFILE="${OUTPUT_DIR}/out.$NOW.txt"
touch $OUTPUTFILE

sbatch --nodes $PARAM_NUMNODES -o $OUTPUTFILE job_TORINO.sbatch
tail -f $OUTPUTFILE
