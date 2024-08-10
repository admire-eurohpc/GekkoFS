#!/bin/bash

rsync -ar --exclude='.git/' ./ mogon-nhr:/lustre/project/nhr-admire/frschimm/wacomm++
