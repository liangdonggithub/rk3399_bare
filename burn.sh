#!/bin/bash 
burn=./tools/rkdeveloptool
loader=./tools/bin/rk3399_loader_v1.26.126.bin
miniloader_addr=0x0
mini_loader=./rk3399_loader.bin

set -e
echo "download loader..."
$burn db $loader
sleep 3
echo "flash os..."
$burn ul $mini_loader
sleep 1
$burn rd

