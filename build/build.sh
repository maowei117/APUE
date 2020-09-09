#! /usr/bin/env bash

if [[ $1 == "clean" ]]; then
    make clean;
    exit 0;
fi 

cmake .;
make -j 3;
