#!/bin/bash

# Settings
path=""

if [ $# -eq 1 ]; then
    path=$1
else
    path="../"
fi

target_output_path="${path}${output_file}"

echo -e "\n=====================BUILD======================\n"

(cd ${path}; make clean; make)
if [ $? -ne 0 ]; then
    (cd ${path}; make clean > /dev/null)
    echo -e "\n=====================FAIL======================\n"
    exit 0
fi

echo -e "\n====================BUILD DONE==================\n"


echo -e "\n======================RUN=======================\n"

(cd ${path}; ./EduBtM_Test)
if [ $? -ne 0 ]; then
    (cd ${path}; make clean > /dev/null)
    echo -e "\n=====================FAIL======================\n"
    exit 0
fi

(cd ${path}; make clean > /dev/null)