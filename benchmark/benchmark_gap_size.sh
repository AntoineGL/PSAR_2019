#!/bin/bash
#benchmark_gap_size.sh

#Ce script fait varier la taille des trous.

if [ "$EUID" -ne 0 ];then
  echo "You must be root to clear page cache."
  exit 1
fi

if [ $# -ne 5 ];then
    echo "Usage $0 <How many occurences>"
    echo "         <How many pages>"
    echo "         <Preread>"
    echo "         <File to write>"
    echo "         <output>"
    exit 1    
fi

declare -a nb=(1 2 3 4 8 12 16 24 32 48 64 80 96 128 192 256 384 512 768 1024 1280 1536 1792 2048 3072 3584 4096 4608 5120 5632 6144 6656 7168 7680 8192 8704 9216 9728 10240)
for gap_size in ${nb[@]};do
    ./benchmark_moyenne.sh "gap=$gap_size" $1 $2 $3 $4 $5
    if [[ $? -ne 0 ]];then
	echo "An error has occured, $0 aborted"
	exit 2
    fi
done
