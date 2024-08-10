#!/bin/bash

#SBATCH -J gkfs_test
#SBATCH -p broadwell
#SBATCH -t 300
#SBATCH -c 36
#SBATCH --oversubscribe
#SBATCH --overcommit

CWD=$(pwd)

PARAMS=""
PARAMS="$PARAMS SLURM_JOB_ID=$SLURM_JOB_ID"
PARAMS="$PARAMS PARAM_HOST=$PARAM_HOST"
PARAMS="$PARAMS PARAM_NUMNODES=$PARAM_NUMNODES"
PARAMS="$PARAMS PARAM_TARGETFS=$PARAM_TARGETFS"

source "${CWD}/spack.source"

FIRST_NODE=$(scontrol show hostname $SLURM_JOB_NODELIST | head -n 1)
echo ssh -o StrictHostKeyChecking=no $FIRST_NODE "$PARAMS $CWD/job.sh"
ssh -o StrictHostKeyChecking=no $FIRST_NODE "$PARAMS $CWD/job.sh"
