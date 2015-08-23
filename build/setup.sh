#!/bin/bash

ln -sf /usr/bin/colorgcc color-gcc
ln -sf /usr/bin/colorgcc color-g++
CC=color-gcc CXX=color-g++ cmake ..

