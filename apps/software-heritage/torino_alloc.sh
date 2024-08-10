#!/bin/bash

salloc --nodes=$1 -t 300 -p broadwell
