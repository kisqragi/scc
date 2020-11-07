#!/bin/bash
./bin/scc test/test.c -o ./bin/tmp.s
gcc -static -o ./bin/tmp ./bin/tmp.s
./bin/tmp
