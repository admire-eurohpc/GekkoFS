#!/bin/bash

rsync -ar --exclude='.git/' ./ torino:~/software-heritage_GKFS
