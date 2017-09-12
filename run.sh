#!/bin/bash
gcc -pthread -g -o Test.o MediaLibrary.c jsmn.c
./Test.o