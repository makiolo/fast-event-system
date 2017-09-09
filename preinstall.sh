#!/bin/bash
if [ -d "concurrentqueue" ]; then
	rm -Rf concurrentqueue
fi
git clone -q https://github.com/cameron314/concurrentqueue

