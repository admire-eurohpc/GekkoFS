#!/bin/bash

#
# Variables, Functions
#

# General parameters
CWD=$(pwd)
NOW=$(date +%Y.%m.%d-%Hh%Mm%Ss)
export SLURM_JOB_ID=${SLURM_JOB_ID:-12345678}

# Experiment parameters
PARAM_HOST="${PARAM_HOST:-MOGONNHR}" # TORINO , MOGONNHR
PARAM_NUMNODES="${PARAM_NUMNODES:-1}"
PARAM_TARGETFS="${PARAM_TARGETFS:-PFS}" # PFS , GKFS

# Paths
if [[ ${PARAM_HOST} == "TORINO" ]]; then
	export PROJECT_DIR="/beegfs/home/frschimm/wacommpp_gkfs"
	export JOBS_DIR="/beegfs/home/frschimm/jobs"
	export APP_DIR=${PROJECT_DIR}/wacommplusplus/build
fi
if [[ ${PARAM_HOST} == "MOGONNHR" ]]; then
	export PROJECT_DIR="/lustre/project/nhr-admire/frschimm/wacomm++"
	export JOBS_DIR="/lustre/project/nhr-admire/frschimm/jobs"
	export APP_DIR="${PROJECT_DIR}/wacommplusplus/build"
fi
OUTPUT_DIR="${PROJECT_DIR}/output"
RESULTFILE="${OUTPUT_DIR}/result.$NOW. txt"

# Output
OUTPUT_FILENAME="${OUTPUT_DIR}/result.${SLURM_JOB_ID}_${NOW}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}_HOST-${PARAM_HOST}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}_NUMNODES-${PARAM_NUMNODES}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}_TARGETFS-${PARAM_TARGETFS}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}.txt"

# App specific
if [[ ${PARAM_HOST} == "TORINO" ]]; then
	export OMP_NUM_THREADS=36
fi
if [[ ${PARAM_HOST} == "MOGONNHR" ]]; then
	export OMP_NUM_THREADS=64
fi

#GKFS paths
export GKFS_ROOT_DIR=/dev/shm/gkfs_rootdir
export GKFS_MOUNT_DIR=/dev/shm/gkfs_mountdir
export GKFS_HOSTFILE=${JOBS_DIR}/gkfs_hostfile-${SLURM_JOB_ID}.txt
if [[ ${PARAM_HOST} == "TORINO" ]]; then
	export GKFS_DIR=${PROJECT_DIR}/gekkofs
	export GKFS_CONF=${PROJECT_DIR}/gkfs.TORINO.conf
	export GKFS_CLIENT=${GKFS_DIR}/install/lib/libgkfs_intercept.so
fi
if [[ ${PARAM_HOST} == "MOGONNHR" ]]; then
	export GKFS_DIR=/home/frschimm/gkfs_nhr/gekkofs
	export GKFS_CONF=${PROJECT_DIR}/gkfs.MOGONNHR.conf
    export GKFS_CLIENT=${GKFS_DIR}/install/lib64/libgkfs_intercept.so
fi

export GKFS_RUN_SCRIPT=${GKFS_DIR}/scripts/run/gkfs

# Other paths
export MPI_HOSTFILE=${JOBS_DIR}/mpi_hostfile-${SLURM_JOB_ID}.txt
if [[ ${PARAM_TARGETFS} == "GKFS" ]]; then
	export PRELOAD_ARG=${GKFS_CLIENT}
fi

# Taskset
CLIENT_TASKSET=""
if [[ ${PARAM_HOST} == "MOGONNHR" ]]; then
	CLIENT_TASKSET="taskset -c 0-63"
fi

# MPI aliases
export ON_FIRST="mpiexec -np 1 --map-by node --hostfile $MPI_HOSTFILE -x LD_PRELOAD=$PRELOAD_ARG -x LIBGKFS_HOSTS_FILE=$GKFS_HOSTFILE -x LD_LIBRARY_PATH"
export ON_ALL="mpiexec -np $PARAM_NUMNODES --map-by node --bind-to none --hostfile $MPI_HOSTFILE -x LD_PRELOAD=$PRELOAD_ARG -x LIBGKFS_HOSTS_FILE=$GKFS_HOSTFILE -x LD_LIBRARY_PATH"


#
# Execution
#

# Paths
mkdir -p "${OUTPUT_DIR}"

# MPI hostfile
rm -f $MPI_HOSTFILE
srun hostname -s | while read line; do echo "${line}"; done >"$MPI_HOSTFILE"
echo cat "$MPI_HOSTFILE"
cat "$MPI_HOSTFILE"
cat $MPI_HOSTFILE | while read line; do
        echo "Running $line"
        ssh -o StrictHostKeychecking=no $line 'hostname' &
done

# Start daemon
if [[ ${PARAM_TARGETFS} == "GKFS" ]]; then
	rm -f $GKFS_HOSTFILE
	srun mkdir -p $GKFS_MOUNT_DIR
	srun mkdir -p $GKFS_ROOT_DIR
	sleep 1
	${GKFS_RUN_SCRIPT} -c ${GKFS_CONF} -v start
	echo cat "$GKFS_HOSTFILE"
	cat "$GKFS_HOSTFILE"
	sleep 1
fi

# Copy input files PFS
if [[ ${PARAM_TARGETFS} == "PFS" ]]; then
	rm -rf ${PROJECT_DIR}/pfs_working_dir/input/*
	rm -rf ${PROJECT_DIR}/pfs_working_dir/processed/*
	rm -rf ${PROJECT_DIR}/pfs_working_dir/output/*
	rm -rf ${PROJECT_DIR}/pfs_working_dir/restart/*
	rm -rf ${PROJECT_DIR}/pfs_working_dir/results/*

	rm -rf ${APP_DIR}/input && ln -sfnv ${PROJECT_DIR}/pfs_working_dir/input ${APP_DIR}/input
	rm -rf ${APP_DIR}/processed && ln -sfnv ${PROJECT_DIR}/pfs_working_dir/processed ${APP_DIR}/processed
	rm -rf ${APP_DIR}/output && ln -sfnv ${PROJECT_DIR}/pfs_working_dir/output ${APP_DIR}/output
	rm -rf ${APP_DIR}/restart && ln -sfnv ${PROJECT_DIR}/pfs_working_dir/restart ${APP_DIR}/restart
	rm -rf ${APP_DIR}/results && ln -sfnv ${PROJECT_DIR}/pfs_working_dir/results ${APP_DIR}/results

	rm -rf ${APP_DIR}/processed/*
	cp ${PROJECT_DIR}/pfs_working_dir/processed.bak/* ${APP_DIR}/processed/

	rm -f ${APP_DIR}/wacomm.json && ln -sf ${PROJECT_DIR}/wacomm.pfs.json ${APP_DIR}/wacomm.json
fi

# Copy input files GKFS
if [[ ${PARAM_TARGETFS} == "GKFS" ]]; then
	$ON_FIRST $CLIENT_TASKSET mkdir $GKFS_MOUNT_DIR/processed
	$ON_FIRST $CLIENT_TASKSET cp -r ${PROJECT_DIR}/pfs_working_dir/processed.bak/* $GKFS_MOUNT_DIR/processed/
	echo " gekkofs ls -l $GKFS_MOUNT_DIR/processed:"
	$ON_FIRST $CLIENT_TASKSET ls -l $GKFS_MOUNT_DIR/processed
	$ON_FIRST $CLIENT_TASKSET mkdir $GKFS_MOUNT_DIR/input
	$ON_FIRST $CLIENT_TASKSET mkdir $GKFS_MOUNT_DIR/output
	$ON_FIRST $CLIENT_TASKSET mkdir $GKFS_MOUNT_DIR/restart
	$ON_FIRST $CLIENT_TASKSET mkdir $GKFS_MOUNT_DIR/results

	rm -f ${APP_DIR}/wacomm.json && ln -sf ${PROJECT_DIR}/wacomm.gkfs.json ${APP_DIR}/wacomm.json
fi

# Other input files
rm -f ${APP_DIR}/sources.json && ln -sf ${PROJECT_DIR}/sources-webinar.modified.json ${APP_DIR}/sources.json

# Run application
cd ${APP_DIR}

echo "(time $ON_ALL --mca orte_base_help_aggregate 0 $CLIENT_TASKSET ./wacommplusplus 2>&1 | tee ${OUTPUT_FILENAME} )"
((time $ON_ALL --mca orte_base_help_aggregate 0 $CLIENT_TASKSET ./wacommplusplus) 2>&1 | tee ${OUTPUT_FILENAME} )
cd ${CWD}

$ON_FIRST $CLIENT_TASKSET du -hs $GKFS_MOUNT_DIR/processed
$ON_FIRST $CLIENT_TASKSET du -hs $GKFS_MOUNT_DIR/input
$ON_FIRST $CLIENT_TASKSET du -hs $GKFS_MOUNT_DIR/output
$ON_FIRST $CLIENT_TASKSET du -hs $GKFS_MOUNT_DIR/restart
$ON_FIRST $CLIENT_TASKSET du -hs $GKFS_MOUNT_DIR/results


scancel ${SLURM_JOB_ID}
