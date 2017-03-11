#!/bin/bash
# Script to compile weight.c

gpio mode 0 in
gpio mode 0 up
gpio mode 1 in
gpio mode 1 up
gpio mode 2 in
gpio mode 2 up


gpio mode 5 out
gpio mode 4 in

gcc ./cjson/cJSON.c weight.c -Wall -o weight -lm -lwiringPi
