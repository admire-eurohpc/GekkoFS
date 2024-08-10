#!/bin/bash
export SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

NODES="1 2 4 8 16 32"
run() {
    NODE_NUMBER=$1
    JOBNAME=$2
    echo "# sbatch -N $NODE_NUMBER -J $JOBNAME ${SCRIPTDIR}/run_gkfs.sbatch "
    sbatch --nodes $NODE_NUMBER -d singleton -J $JOBNAME --output=/dev/null -p broadwell -t 60 ${SCRIPTDIR}/run_gkfs_wrapper.sbatch $SCRIPTDIR > /dev/null
}

for NODE in $NODES
do
    echo "# sbatch for node num: $NODE"
	run $NODE "GekkoFS_Nek"
done
