#!/bin/bash
export SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

NODES="8 8 8 8 8 8"
run() {
    NODE_NUMBER=$1
    JOBNAME=$2
    sbatch --nodes $NODE_NUMBER -J $JOBNAME --output=/dev/null -p broadwell -t 60 ${SCRIPTDIR}/run_beegfs_wrapper.sbatch $SCRIPTDIR > /dev/null
}

for NODE in $NODES
do
    echo "# sbatch for node num: $NODE"
	run $NODE "BeeGFS_Nek"
done
