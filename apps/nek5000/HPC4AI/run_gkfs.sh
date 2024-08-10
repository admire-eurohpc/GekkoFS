#!/bin/bash
JOBOUTDIR=$1

# SETUP
WORKDIR=/beegfs/home/m.vef/run/job_tmp/${SLURM_JOB_ID}
mkdir -p $WORKDIR

SLURM_HOSTS=$(squeue -j $SLURM_JOB_ID -o "%N" | tail -n +2)
scontrol show hostname $SLURM_HOSTS | sort -u > $WORKDIR/hostfile_mpi

# dont ask
cat $WORKDIR/hostfile_mpi | while read line
do
    #echo "Running $line"
    ssh -o StrictHostKeychecking=no $line 'hostname' 2>&1 > /dev/null &
done
wait

# RUN
~/gekkofs/scripts/run/gkfs -c ~/run/gkfs_sockets.conf -v start 
source ~/run/gkfs_sockets.conf
srun mkdir -p /dev/shm/vef_gkfs_mountdir
echo "copying input..."
export LIBGKFS_HOSTS_FILE=$LIBGKFS_HOSTS_FILE
LD_PRELOAD=${PRELOAD_LIB} cp /beegfs/home/m.vef/admire/turbPipe/run_gkfs/input/* /dev/shm/vef_gkfs_mountdir/
cd /beegfs/home/m.vef/admire/turbPipe/run_gkfs

NODE_NUM=$(wc -l $WORKDIR/hostfile_mpi | awk '{print $1}')
PROC_NUM=$(( $NODE_NUM*16 ))
#JOBOUTDIR="/beegfs/home/m.vef/run/job_out/nek_${SLURM_JOB_ID}_${NODE_NUM}n_${PROC_NUM}p"
#mkdir -p $JOBOUTDIR
JOBOUTFILE=$JOBOUTDIR/nek.out

echo "Starting Nek5000 on $NODE_NUM nodes (16 processes each $PROC_NUM total) on GekkoFS ..."
echo "Check $JOBOUTFILE for progress"
start_time=$(date +%s.%N)  # Get seconds since epoch with nanoseconds
time mpiexec -np $PROC_NUM --oversubscribe --map-by node --hostfile $WORKDIR/hostfile_mpi -x LIBGKFS_HOSTS_FILE=${LIBGKFS_HOSTS_FILE} -x LD_PRELOAD=${PRELOAD_LIB} -x  LD_LIBRARY_PATH=${LD_LIBRARY_PATH} ./nek5000 > $JOBOUTFILE
end_time=$(date +%s.%N)
elapsed_time=$(echo "$end_time - $start_time" | bc)
echo "Elapsed time: $elapsed_time seconds"

# cleanup 
~/gekkofs/scripts/run/gkfs -c ~/run/gkfs_sockets.conf stop

rm -rf $WORKDIR