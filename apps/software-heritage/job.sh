#!/bin/bash

#
# Variables, Functions
#

# General parameters
CWD=$(pwd)
NOW=$(date +%Y.%m.%d-%Hh%Mm%Ss)
export SLURM_JOB_ID=${SLURM_JOB_ID:-12345678}

# Experiment parameters
PARAM_HOST="${PARAM_HOST:-TORINO}" # TORINO , MOGONNHR
PARAM_NUMNODES="${PARAM_NUMNODES:-8}"
PARAM_TARGETFS="${PARAM_TARGETFS:-PFS}" # PFS , GKFS

# Paths
if [[ ${PARAM_HOST} == "TORINO" ]]; then
	export PROJECT_DIR="/beegfs/home/frschimm/software-heritage_GKFS"
	export JOBS_DIR="/beegfs/home/frschimm/jobs"
	export APP_DIR="${PROJECT_DIR}/Software-Heritage-Analytics"
fi
if [[ ${PARAM_HOST} == "MOGONNHR" ]]; then
	export PROJECT_DIR="/lustre/project/nhr-admire/frschimm/"
	export JOBS_DIR="/lustre/project/nhr-admire/frschimm/jobs"
	export APP_DIR="${PROJECT_DIR}/Software-Heritage-Analytics"
fi
OUTPUT_DIR="${PROJECT_DIR}/output"
RESULTFILE="${OUTPUT_DIR}/result.$NOW. txt"

# Output
OUTPUT_FILENAME="${OUTPUT_DIR}/result.${SLURM_JOB_ID}_${NOW}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}_HOST-${PARAM_HOST}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}_NUMNODES-${PARAM_NUMNODES}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}_TARGETFS-${PARAM_TARGETFS}"
OUTPUT_FILENAME="${OUTPUT_FILENAME}.txt"


#GKFS paths
export GKFS_ROOT_DIR=/dev/shm/gkfs_rootdir
export GKFS_MOUNT_DIR=/dev/shm/gkfs_mountdir
export GKFS_HOSTFILE=${JOBS_DIR}/gkfs_hostfile-${SLURM_JOB_ID}.txt
if [[ ${PARAM_HOST} == "TORINO" ]]; then
	export GKFS_DIR="/beegfs/home/frschimm/wacommpp_gkfs/gekkofs"
	export GKFS_CONF=${PROJECT_DIR}/gkfs.TORINO.conf
	export GKFS_CLIENT=${GKFS_DIR}/install/lib/libgkfs_intercept.so
fi
if [[ ${PARAM_HOST} == "MOGONNHR" ]]; then
	export GKFS_DIR="/lustre/project/nhr-admire/frschimm/wacomm++/gekkofs"
	export GKFS_CONF=${PROJECT_DIR}/gkfs.MOGONNHR.conf
    export GKFS_CLIENT=${GKFS_DIR}/install/lib64/libgkfs_intercept.so
fi

export GKFS_RUN_SCRIPT=${GKFS_DIR}/scripts/run/gkfs

# Application specific paths
export SPARK_APP_PATH=/beegfs/home/frschimm/software-heritage_GKFS/Software-Heritage-Analytics/Example/

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
export ON_FIRST="mpiexec -np 1 --map-by node --hostfile $MPI_HOSTFILE -x LD_PRELOAD=$PRELOAD_ARG -x LIBGKFS_HOSTS_FILE=$GKFS_HOSTFILE -x LD_LIBRARY_PATH $CLIENT_TASKSET"
export ON_ALL="mpiexec -np $PARAM_NUMNODES --map-by node --bind-to none --hostfile $MPI_HOSTFILE -x LD_PRELOAD=$PRELOAD_ARG -x LIBGKFS_HOSTS_FILE=$GKFS_HOSTFILE -x LD_LIBRARY_PATH $CLIENT_TASKSET"


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

# Run application
cd ${APP_DIR}/Tools
echo First node: $(hostname)

echo Run sha:
export ENABLE_CACHEMIRE=0
export CACHEMIRE_LD_PRELOAD_PATH=""
export CACHEMIRE_DATA_DIR="${APP_DIR}s/Example/CACHE/"
if [[ ${PARAM_TARGETFS} == "GKFS" ]]; then
  export CACHEMIRE_LD_PRELOAD_PATH=$GKFS_CLIENT
	export CACHEMIRE_DATA_DIR=$GKFS_MOUNT_DIR
  export LIBGKFS_HOSTS_FILE=$GKFS_HOSTFILE
fi

# mpiexec -np $PARAM_NUMNODES --map-by node --bind-to none --hostfile $MPI_HOSTFILE \
#   -x ENABLE_CACHEMIRE \
#   -x CACHEMIRE_LD_PRELOAD_PATH\
#   -x CACHEMIRE_DATA_DIR \
#   -x LIBGKFS_HOSTS_FILE=$GKFS_HOSTFILE \
#   -x LD_LIBRARY_PATH \
#   $CLIENT_TASKSET \
#   bash sha_on_slurm.sh
#
srun $CLIENT_TASKSET bash sha_on_slurm.sh &

echo Dashboard Client:

dashboardclient -a $(hostname) \
  -p 4320 \
  -r ${SPARK_APP_PATH}/recipe_test.json \
  -m $(hostname) \
  -n licensectrl \
  -dir ${SPARK_APP_PATH}/recipe_test \
  -sb scancode -si ${SPARK_APP_PATH}/scancode_index.json \
  -rp ${SPARK_APP_PATH}/ramdisk \
  -gp ${SPARK_APP_PATH}/grafo.txt \
  -op ${SPARK_APP_PATH}/output \
  -D

# echo "(time $ON_ALL --mca orte_base_help_aggregate 0 $CLIENT_TASKSET ./wacommplusplus 2>&1 | tee ${OUTPUT_FILENAME} )"
# ((time $ON_ALL --mca orte_base_help_aggregate 0 $CLIENT_TASKSET ./wacommplusplus) 2>&1 | tee ${OUTPUT_FILENAME} )
cd ${CWD}

scancel ${SLURM_JOB_ID}
