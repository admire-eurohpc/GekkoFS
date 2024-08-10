#!/bin/bash

# node this needs to be run inside a job as an alternative to sched_lustre.sh

./run_lustre.sh /lustre/project/nhr-admire/vef/run/nek5000/job_out/nek_8n_16p 8
./run_lustre.sh /lustre/project/nhr-admire/vef/run/nek5000/job_out/nek_4n_16p 4
./run_lustre.sh /lustre/project/nhr-admire/vef/run/nek5000/job_out/nek_2n_16p 2
./run_lustre.sh /lustre/project/nhr-admire/vef/run/nek5000/job_out/nek_1n_16p 1
