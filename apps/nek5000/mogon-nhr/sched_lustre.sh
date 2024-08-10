#!/bin/bash
export SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

NODES="1"
run() {
    NODE_NUMBER=$1
    JOBNAME=$2
    #sbatch -N $NODE_NUMBER -d singleton -J $JOBNAME --output=/dev/null --overcommit --oversubscribe --partition parallel -A nhr-admire -t 60 ${SCRIPTDIR}/run_gkfs_wrapper.sbatch $SCRIPTDIR > /dev/null
    sbatch -N $NODE_NUMBER -d singleton -J $JOBNAME --overcommit --oversubscribe --partition parallel -A nhr-admire -t 60 ${SCRIPTDIR}/run_lustre_wrapper.sbatch $SCRIPTDIR
}

for NODE in $NODES
do
    echo "# sbatch for node num: $NODE"
        run $NODE "GekkoFS_Lustre"
done
