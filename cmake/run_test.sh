#!/bin/bash

echo Running test $1 with coverage
export ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer-3.6
export ASAN_OPTIONS=check_initialization_order=1
rm $1.coverage 2> /dev/null
rm $1.gcno 2> /dev/null
rm default.profraw 2> /dev/null
./$1
ret=$?
llvm-profdata-3.6 merge -o $1.gcno default.profraw 2> /dev/null
llvm-cov-3.6 show ./$1 -instr-profile=$1.gcno > $1.coverage
exit $ret

