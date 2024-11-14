#!/bin/bash

CWD=$(pwd)

export LD_LIBRARY_PATH=${CWD}:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=${CWD}/rlib/include:$LD_LIBRARY_PATH
