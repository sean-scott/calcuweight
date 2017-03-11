#!/bin/bash

gpio mode 0 in
gpio mode 0 up

#./bin/server 7070 & sudo ./weight &
sudo ./weight & ./bin/server 7070
