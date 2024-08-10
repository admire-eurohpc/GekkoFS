#!/bin/bash

#salloc -N 2 -t 360 -p parallel -A m2_zdvresearch -C skylake
salloc -N 4 -t 60 -p parallel -A nhr-admire --oversubscribe --overcommit