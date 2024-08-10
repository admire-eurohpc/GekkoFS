#!/bin/bash

#SBATCH -J gkfs_test
#SBATCH -A nhr-admire
#SBATCH -p largemem
#SBATCH -t 60
#SBATCH -c 128
#SBATCH --oversubscribe
#SBATCH --overcommit

CWD=$(pwd)

PARAMS=""
PARAMS="$PARAMS SLURM_JOB_ID=$SLURM_JOB_ID"
PARAMS="$PARAMS PARAM_HOST=$PARAM_HOST"
PARAMS="$PARAMS PARAM_NUMNODES=$PARAM_NUMNODES"
PARAMS="$PARAMS PARAM_TARGETFS=$PARAM_TARGETFS"

FIRST_NODE=$(scontrol show hostname $SLURM_JOB_NODELIST | head -n 1)
echo ssh -o StrictHostKeyChecking=no $FIRST_NODE "$PARAMS $CWD/job.sh"
ssh -o StrictHostKeyChecking=no $FIRST_NODE "$PARAMS $CWD/job.sh"
