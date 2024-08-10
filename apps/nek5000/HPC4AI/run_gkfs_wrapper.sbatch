#!/bin/bash
SCRIPTDIR=$1

SLURM_HOSTS=$(squeue -j $SLURM_JOB_ID -o "%N" | tail -n +2)
NODE_NUM=$(scontrol show hostname $SLURM_HOSTS | wc -l)
PROC_NUM=$(( $NODE_NUM*16 ))
JOBOUTDIR="/beegfs/home/m.vef/run/job_out/nek_${NODE_NUM}n_${PROC_NUM}p_${SLURM_JOB_ID}"
mkdir -p $JOBOUTDIR

#execute
$SCRIPTDIR/run_gkfs.sh $JOBOUTDIR >> "${JOBOUTDIR}/slurm.out"
