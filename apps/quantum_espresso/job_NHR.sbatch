#!/bin/bash

#SBATCH -J quesgkfs
#SBATCH -A nhr-admire
#SBATCH -p parallel
#SBATCH -t 60
#SBATCH -c 128
#SBATCH --oversubscribe
#SBATCH --overcommit

CWD=$(pwd)

PARAMS="LOG_DIR=$LOG_DIR"
PARAMS="$PARAMS NODE_NUMBER=$NODE_NUMBER"
PARAMS="$PARAMS SLURM_JOB_ID=$SLURM_JOB_ID"
PARAMS="$PARAMS TARGET=$TARGET"
PARAMS="$PARAMS LOCAL_DEVICE=$LOCAL_DEVICE"

FIRST_NODE=$(scontrol show hostname $SLURM_JOB_NODELIST | head -n 1)
echo ssh -o StrictHostKeyChecking=no $FIRST_NODE "$PARAMS PARAMS_STR=\"$PARAMS\" $CWD/job.sh" |& tee "$LOG_DIR/$SLURM_JOB_ID.out"
ssh -o StrictHostKeyChecking=no $FIRST_NODE "$PARAMS PARAMS_STR=\"$PARAMS\" $CWD/job.sh" |& tee "$LOG_DIR/$SLURM_JOB_ID.out"
