#!/bin/bash

echo "QuEs Job start..."

echo "Params set from outside:"
echo "\"$PARAMS_STR\""

###
### Experiment parameters
###

export NODE_NUMBER=${NODE_NUMBER:-4}
export SLURM_JOB_ID=${SLURM_JOB_ID:-12345678}
TARGET="${TARGET:-GKFS}"            # PFS GKFS
LOCAL_DEVICE="${LOCAL_DEVICE:-SSD}" # RAMFS SSD
USE_DARSHAN="${USE_DARSHAN:-FALSE}"

echo "NODE_NUMBER=$NODE_NUMBER SLURM_JOB_ID=$SLURM_JOB_ID TARGET=$TARGET LOCAL_DEVICE=$LOCAL_DEVICE"

NOW=$(date +%Y.%m.%d_%Hh%Mm%Ss)

export LOG_DIR=${LOG_DIR:-/home/frschimm/QuEsGKFS/output}
echo LOG_DIR $LOG_DIR
mkdir -p $LOG_DIR &>/dev/null

#DDIST_TEMP_DIR="/home/frschimm/gkfstemp"
DIST_TEMP_DIR="/lustre/project/nhr-admire/frschimm/jobg/gkfstemp"
export JOB_TEMP_DIR=$DIST_TEMP_DIR/slurm-$SLURM_JOB_ID
mkdir -p $JOB_TEMP_DIR

#srun rm -rf /localscatch/$SLURM_JOB_ID/*

###
### Hostfiles
###
echo Hostfiles

echo "MPI_HOSTFILE:"
MPI_HOSTFILE=~/temp/hosts_mpi.$SLURM_JOB_ID
srun hostname -s | while read line; do echo "${line}"; done >"$MPI_HOSTFILE"
cat "$MPI_HOSTFILE"

cat $MPI_HOSTFILE | while read line; do
	echo "Running $line"
	ssh -o StrictHostKeychecking=no $line 'hostname' &
done

###
### Basics
###
echo Basics

ON_ALL="mpiexec -np $((NODE_NUMBER * 16)) --map-by node --hostfile $MPI_HOSTFILE --verbose"

###
### GKFS Preparation
###
echo GKFS Preparation

if [[ $TARGET == "GKFS" ]]; then
	export GKFS_MOUNT_DIR=/dev/shm/gkfs_mountdir
	if [[ $LOCAL_DEVICE == "SSD" ]]; then
		export GKFS_ROOT_DIR=/localscratch/$SLURM_JOB_ID/gkfs_rootdir
	fi
	if [[ $LOCAL_DEVICE == "RAMFS" ]]; then
		export GKFS_ROOT_DIR=/dev/shm/gkfs_rootdir
	fi

	export LIBGKFS_PRELOAD_LIB=/home/frschimm/gkfs_nhr/gekkofs/install/lib64/libgkfs_intercept.so
	echo 1 $LIBGKFS_PRELOAD_LIB
	export LIBGKFS_HOSTS_FILE=${JOB_TEMP_DIR}/gkfs_hostfile
	export LIBGKFS_LOG=info,errors,warnings
	export LIBGKFS_LOG_OUTPUT=/dev/shm/frschimm_gkfs_client.log
fi

###
### Other Preparation
###
echo Other Preparation

if [[ $TARGET == "PFS" ]]; then
	export WORKING_PATH=/home/frschimm/questemp
fi
if [[ $TARGET == "GKFS" ]]; then
	export WORKING_PATH="${GKFS_MOUNT_DIR}/"
fi

###
### Start GKFS Daemon
###

if [[ $TARGET == "GKFS" ]]; then

	# echo "Run GKFS daemon..."

	# GKFS_DAEMON_LOG_PATH=/dev/shm/gkfs_daemon_$(date +%Y.%m.%d_%Hh%Mm%Ss).log \
	# srun --disable-status -N $NODE_NUMBER --ntasks=$NODE_NUMBER --ntasks-per-node=1 \
	# --overcommit \
	# --oversubscribe \
	# --cpus-per-task=64 \
	# numactl --cpunodebind=1 --membind=1 $GKFS_DAEMON_PATH \
	# -r $GKFS_ROOT_DIR \
	# -m $GKFS_MOUNT_DIR \
	# -H $GKFS_HOST_FILE \
	# -l ib0 \
	# -P ofi+verbs --auto-sm &

	/home/frschimm/gkfs_nhr/gekkofs/scripts/run/gkfs -c /home/frschimm/QuEsGKFS/gkfs.conf -v start

	echo 2 $LIBGKFS_PRELOAD_LIB

	sleep 2

	# echo "GKFS_HOST_FILE:"
	# cat $GKFS_HOST_FILE

fi

###
### Run
###

CWD=$(pwd)
cd /home/frschimm/quantumespresso_gkfs/q-e/PHonon/examples/tetra_example

echo "Run:"

LOG_PREFIX=$TARGET-$LOCAL_DEVICE-$NODE_NUMBER

LD_PRELOAD_ARG=
if [[ $TARGET == "GKFS" ]]; then
	LD_PRELOAD_ARG=$LIBGKFS_PRELOAD_LIB
fi
if [[ $USE_DARSHAN == "TRUE" ]]; then
	export DARSHAN_CONFIG_PATH=/home/frschimm/QuEsGKFS/darshan.config
	#export DARSHAN_DISABLE_SHARED_REDUCTION=1
	#export DXT_ENABLE_IO_TRACE=1
	LD_PRELOAD_ARG=/home/frschimm/QuEsGKFS/darshan_install/lib/libdarshan.so
fi

# if [[ $TARGET == "GKFS" ]]; then
#     export LD_LIBRARY_PATH=$GKFS_DEPS_PATH/install/lib:$GKFS_DEPS_PATH/install/lib64:$GKFS_PATH/install/lib64:$LD_LIBRARY_PATH
# fi

export QUES_TMP_DIR=$WORKING_PATH

if [[ $TARGET == "PFS" ]]; then
	mkdir -p $QUES_TMP_DIR &>/dev/null
	echo ls -la $QUES_TMP_DIR
	ls -la $QUES_TMP_DIR
fi

echo 3 $LIBGKFS_PRELOAD_LIB

export QUES_MPI_CMD="$ON_ALL \
-x LIBGKFS_LOG_OUTPUT=$LIBGKFS_LOG_OUTPUT -x LIBGKFS_LOG=$LIBGKFS_LOG -x LIBGKFS_LOG_PER_PROCESS="1" \
-x LD_PRELOAD=$LD_PRELOAD_ARG -x LIBGKFS_HOSTS_FILE=$LIBGKFS_HOSTS_FILE -x LD_LIBRARY_PATH taskset -c 0-63"

echo 4 $LIBGKFS_PRELOAD_LIB

echo QUES_MPI_CMD: $QUES_MPI_CMD

(time ./run_example) |& tee "$LOG_DIR/${SLURM_JOB_ID}_${NOW}_${LOG_PREFIX}_measure.timing.txt"

cd $CWD

echo "Job done"

#scancel $SLURM_JOB_ID
