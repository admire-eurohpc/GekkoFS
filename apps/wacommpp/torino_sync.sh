#!/bin/bash

rsync -ar --exclude='.git/' ./ torino:~/wacommpp_gkfs
