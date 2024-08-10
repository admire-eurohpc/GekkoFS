#!/bin/bash

CWD=$(pwd)

export OUTPUT_DIR=$CWD/output
mkdir -p "${OUTPUT_DIR}"

NOW=$(date +%Y.%m.%d-%Hh%Mm%Ss)
OUTPUTFILE="${OUTPUT_DIR}/out.$NOW.txt"
touch $OUTPUTFILE

JOB_ID=""

for PARAM_TARGETFS in "GKFS" "PFS"; do
# for PARAM_TARGETFS in "GKFS"; do
    export PARAM_TARGETFS

     # for PARAM_NUMNODES in "1" "2" "4" "8" "16" "32"; do
     for PARAM_NUMNODES in "1" "2" "4" "8" "16"; do
     # for PARAM_NUMNODES in "2" ; do
        export PARAM_NUMNODES

        JOB_DEP=""
        if [[ ${JOB_ID} != "" ]]; then
            JOB_DEP="--dependency=afterany:${JOB_ID}"
        fi

        JOB_ID=$(sbatch --nodes $PARAM_NUMNODES --parsable $JOB_DEP -o $OUTPUTFILE job_NHR.sbatch)

    done
done

echo Jobs sheduled. Output:
echo ${OUTPUTFILE}
# tail -f ${OUTPUTFILE}
