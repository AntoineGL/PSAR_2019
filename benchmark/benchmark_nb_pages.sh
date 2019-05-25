#!/bin/bash
#benchmark_nb_pages.sh

#Ce script fait varier le nombre de pages écrites.

if [ $# -ne 6 ];then
    echo "Usage $0 <type (sequential | random | gap=<gap size> | oneshot)"
    echo "         <How many occurences>"
    echo "         <Max nb_pages (2^n)>"
    echo "         <Preread>"
    echo "         <File to write>"
    echo "         <output>"
    exit 1    
fi

declare -a nb=(1 2 3 4 8 12 16 24 32 48 64 80 96 128 192 256 384 512 768 1024 1536 2048 3072 4096 5120 6144 7168 8192 10144 12192 14240 16384 )
for $nb_pages in ${nb[@]};do
    ./benchmark_moyenne.sh $1 $2 $nb_pages $4 $5 $6
    if [[ $? -ne 0 ]];then
	echo "An error has occured, $0 aborted"
	exit 2
    fi
done



      
