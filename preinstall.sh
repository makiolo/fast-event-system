#!/bin/bash

mkdir -p include
cd include

if [ -d "concurrentqueue" ]; then
	rm -Rf concurrentqueue
fi
git clone -q https://github.com/cameron314/concurrentqueue

if [ -d "readerwriterqueue" ]; then
	rm -Rf readerwriterqueue
fi
git clone -q https://github.com/cameron314/readerwriterqueue
