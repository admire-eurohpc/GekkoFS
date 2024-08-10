#!/bin/bash

echo "vpicio Job start..."

echo "Params set from outside:"
echo "\"$PARAMS_STR\""

###
### Experiment Parameters
###

export NODE_NUMBER=${NODE_NUMBER:-2}
SLURM_JOB_ID=${SLURM_JOB_ID:-12345678}
TARGET="${TARGET:-GKFS}" # PFS GKFS
LOCAL_DEVICE="${LOCAL_DEVICE:-RAMFS}" # RAMFS SSD
USE_DARSHAN="${USE_DARSHAN:-FALSE}"

echo NODE_NUMBER=$NODE_NUMBER SLURM_JOB_ID=$SLURM_JOB_ID TARGET=$TARGET LOCAL_DEVICE=$LOCAL_DEVICE

NOW=$(date +%Y.%m.%d_%Hh%Mm%Ss)

export LOG_DIR=${LOG_DIR:-/home/frschimm/vpicioGKFS/output}
echo LOG_DIR $LOG_DIR
mkdir -p $LOG_DIR &>/dev/null

DIST_TEMP_DIR="/home/frschimm/gkfstemp"

###
### Hostfiles
###
echo Hostfiles

echo "MPI_HOSTFILE:"
MPI_HOSTFILE=~/temp/hosts_mpi.$SLURM_JOB_ID
srun hostname -s | while read line; do echo "${line}"; done > "$MPI_HOSTFILE" ; cat "$MPI_HOSTFILE"

###
### Basics
###
echo Basics

ON_ALL="mpiexec -np $(( NODE_NUMBER * 32 )) --map-by node --hostfile $MPI_HOSTFILE --verbose"

###
### GKFS Preparation
###
echo GKFS Preparation

if [[ $TARGET == "GKFS"  ]]; then
    GKFS_MOUNT_DIR=/dev/shm/gkfs_mountdir
    if [[ $LOCAL_DEVICE == "SSD" ]]; then
        GKFS_ROOT_DIR=/localscratch/$SLURM_JOB_ID/gkfs_rootdir
    fi
    if [[ $LOCAL_DEVICE == "RAMFS" ]]; then
        GKFS_ROOT_DIR=/dev/shm/gkfs_rootdir
    fi
    
    export JOB_TEMP_DIR=$DIST_TEMP_DIR/slurm-$SLURM_JOB_ID
    rm -rf $JOB_TEMP_DIR
    mkdir -p $JOB_TEMP_DIR

    export GKFS_HOST_FILE="$JOB_TEMP_DIR/gkfs_hostfile_$SLURM_JOB_ID"

    export GKFS_PATH=/home/frschimm/QuEsGKFS/gekkofs/

    export GKFS_DAEMON_PATH=$GKFS_PATH/install/bin/gkfs_daemon
    export GKFS_CLIENT_PATH=$GKFS_PATH/install/lib64/libgkfs_intercept.so

    echo GKFS_DAEMON_PATH $GKFS_DAEMON_PATH
    echo GKFS_CLIENT_PATH $GKFS_CLIENT_PATH

    export LIBGKFS_LOG=none
    export LIBGKFS_LOG_OUTPUT=/dev/shm/gkfs_client_$NOW.log
fi


###
### Other Preparation
###
echo Other Preparation

if [[ $TARGET == "PFS" ]]; then
    export WORKING_PATH=/lustre/project/zdvresearch/frschimm/vpiciotemp
    rm -rf $WORKING_PATH
    mkdir -p $WORKING_PATH
fi
if [[ $TARGET == "GKFS" ]]; then
    export WORKING_PATH="${GKFS_MOUNT_DIR}/"
fi


###
### Start GKFS Daemon
###


if [[ $TARGET == "GKFS" ]]; then

    echo "Run GKFS daemon..."
    
    GKFS_DAEMON_LOG_PATH=/dev/shm/gkfs_daemon_$(date +%Y.%m.%d_%Hh%Mm%Ss).log \
    srun --disable-status -N $NODE_NUMBER --ntasks=$NODE_NUMBER --ntasks-per-node=1 \
    --overcommit \
    --oversubscribe \
    --cpus-per-task=64 \
    numactl --cpunodebind=1 --membind=1 $GKFS_DAEMON_PATH \
    -r $GKFS_ROOT_DIR \
    -m $GKFS_MOUNT_DIR \
    -H $GKFS_HOST_FILE \
    -l ib0 \
    -P ofi+sockets --auto-sm &

    sleep 20

    echo "GKFS_HOST_FILE:"
    cat $GKFS_HOST_FILE

fi

###
### Run
###

CWD=$(pwd)
# cd /home/frschimm/vpiciotemp/vpic-io

echo "Run:"

LOG_PREFIX=$TARGET-$LOCAL_DEVICE-$NODE_NUMBER

LD_PRELOAD_ARG=
if [[ $TARGET == "GKFS" ]]; then
    LD_PRELOAD_ARG=$GKFS_CLIENT_PATH
fi
if [[ $USE_DARSHAN == "TRUE"  ]]; then
    export DARSHAN_CONFIG_PATH=/home/frschimm/QuEsGKFS/darshan.config
    #export DARSHAN_DISABLE_SHARED_REDUCTION=1
    #export DXT_ENABLE_IO_TRACE=1
    LD_PRELOAD_ARG=/home/frschimm/QuEsGKFS/darshan_install/lib/libdarshan.so
fi

if [[ $TARGET == "GKFS" ]]; then
    export LD_LIBRARY_PATH=$GKFS_DEPS_PATH/install/lib:$GKFS_DEPS_PATH/install/lib64:$GKFS_PATH/install/lib64:$LD_LIBRARY_PATH
fi

export QUES_TMP_DIR=$WORKING_PATH

if [[ $TARGET == "PFS" ]]; then
    mkdir -p $QUES_TMP_DIR &>/dev/null
    echo ls -la $QUES_TMP_DIR
    ls -la $QUES_TMP_DIR
fi

# export QUES_MPI_CMD="$ON_ALL \
# -x LIBGKFS_LOG_OUTPUT=$LIBGKFS_LOG_OUTPUT -x LIBGKFS_LOG=$LIBGKFS_LOG \
# -x LD_PRELOAD=$LD_PRELOAD_ARG -x LIBGKFS_HOSTS_FILE=$GKFS_HOST_FILE -x LD_LIBRARY_PATH"
# echo QUES_MPI_CMD: $QUES_MPI_CMD

# # cfg/h5bench1m.cfg
# salloc -p 40n --nodelist=ec[01-32] -N 32 --ntasks-per-node=16 /usr/mpi/gcc/openmpi-4.1.5a1/bin/mpirun --allow-run-as-root /work/qian/vpic/h5bench/build/h5bench_write /work/qian/vpic/cfg/h5bench1m.cfg /exafs/s8/test.1m.h5 | tee result/lustre/bio.w1m2s.log
# salloc -p 40n --nodelist=ec[01-32] -N 32 --ntasks-per-node=16 /usr/mpi/gcc/openmpi-4.1.5a1/bin/mpirun --allow-run-as-root /work/qian/vpic/h5bench/build/h5bench_read /work/qian/vpic/cfg/h5bench1m.cfg /exafs/s8/test.1m.h5 | tee result/lustre/bio.r1m2s.log

# # cfg/h5bench32k.cfg
# salloc -p 40n --nodelist=ec[01-32] -N 32 --ntasks-per-node=16 /usr/mpi/gcc/openmpi-4.1.5a1/bin/mpirun --allow-run-as-root /work/qian/vpic/h5bench/build/h5bench_write /work/qian/vpic/cfg/h5bench32k.cfg /exafs/s8/test.32k.h5 | tee result/lustre/bio.w32k2s.log
# salloc -p 40n --nodelist=ec[01-32] -N 32 --ntasks-per-node=16 /usr/mpi/gcc/openmpi-4.1.5a1/bin/mpirun --allow-run-as-root /work/qian/vpic/h5bench/build/h5bench_read /work/qian/vpic/cfg/h5bench32k.cfg /exafs/s8/test.32k.h5 | tee result/lustre/bio.r32k2s.log

MPI_GKFS_ARGS=""
if [[ $TARGET == "GKFS" ]]; then
    MPI_GKFS_ARGS="-x LIBGKFS_LOG_OUTPUT=$LIBGKFS_LOG_OUTPUT -x LIBGKFS_LOG=$LIBGKFS_LOG -x LD_PRELOAD=$LD_PRELOAD_ARG -x LIBGKFS_HOSTS_FILE=$GKFS_HOST_FILE -x LD_LIBRARY_PATH"
fi

( time mpirun --hostfile $MPI_HOSTFILE -np $(( NODE_NUMBER * 16 )) --map-by node $MPI_GKFS_ARGS \
    /home/frschimm/vpicioGKFS/h5bench/build/h5bench_write /home/frschimm/vpicioGKFS/h5bench1m.cfg $WORKING_PATH/test.1m.h5 ) \
    |& tee "$LOG_DIR/${SLURM_JOB_ID}_write_1m_${NODE_NUMBER}_${TARGET}_${NOW}_${LOG_PREFIX}_measure.timing.txt"


cd $CWD

echo "Job done"

#scancel $SLURM_JOB_ID
