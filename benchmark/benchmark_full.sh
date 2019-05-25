#!/bin/bash
#benchmark_full.sh

if [ "$EUID" -ne 0 ];then
  echo "You must be root to use hdparm and to clear page cache."
  exit 1
fi

if [ $# -lt 3 ];then
    echo "Usage $0 <File to write>"
    echo "         <Disk name ex: /dev/sdb>"
    echo "         <How many occurences>"
    echo "         <Prefix to file (default:none)>"
    exit 1
fi

file=$1
disk=$2
occ=$3
prefix=$4


hdparm -W 1 $disk

if [ $? -ne 0 ];then
    echo "erreur hdparm 1"
    exit 2
fi

echo "Test 1/12: File: ${prefix}DC1_read1_sequential_${occ}occ.csv"
./benchmark_nb_pages.sh sequential $occ 1024 1 $file ${prefix}DC1_read1_sequential_${occ}occ.csv

echo "Test 2/12: File: ${prefix}DC1_read1_random_${occ}occ.csv"
./benchmark_nb_pages.sh random $occ 1024 1 $file ${prefix}DC1_read1_random_${occ}occ.csv

echo "Test 3/12: File: ${prefix}DC1_read1_gap1_${occ}occ.csv"
./benchmark_nb_pages.sh "gap=1" $occ 1024 1 $file ${prefix}DC1_read1_gap1_${occ}occ.csv

echo "Test 4/12: File: ${prefix}DC1_read2_gap1_${occ}occ.csv"
./benchmark_nb_pages.sh "gap=1" $occ 1024 2 $file ${prefix}DC1_read2_gap1_${occ}occ.csv

echo "Test 5/12: File: ${prefix}DC1_read1_gap_200pages_${occ}occ.csv"
./benchmark_gap_size.sh $occ 200 1000 1 $file ${prefix}DC1_read1_gap_200pages_${occ}occ.csv

echo "Test 6/12: File: ${prefix}DC1_read2_gap_200pages_${occ}occ.csv"
./benchmark_gap_size.sh $occ 200 1000 2 $file ${prefix}DC1_read2_gap_200pages_${occ}occ.csv


hdparm -W 0 $disk

if [ $? -ne 0 ];then
    echo "erreur hdparm 0"
    exit 2
fi
echo "Test 7/12: File: ${prefix}DC0_read1_sequential_${occ}occ.csv"
./benchmark_nb_pages.sh sequential $occ 1024 1 $file ${prefix}DC0_read1_sequential_${occ}occ.csv

echo "Test 8/12: File: ${prefix}DC0_read1_random_${occ}occ.csv"
./benchmark_nb_pages.sh random $occ 1024 1 $file ${prefix}DC0_read1_random_${occ}occ.csv

echo "Test 9/12: File: ${prefix}DC0_read1_gap1_${occ}occ.csv"
./benchmark_nb_pages.sh "gap=1" $occ 1024 1 $file ${prefix}DC0_read1_gap1_${occ}occ.csv

echo "Test 10/12: File: ${prefix}DC0_read2_gap1_${occ}occ.csv"
./benchmark_nb_pages.sh "gap=1" $occ 1024 2 $file ${prefix}DC0_read2_gap1_${occ}occ.csv

echo "Test 11/12: File: ${prefix}DC0_read1_gap_200pages_${occ}occ.csv"
./benchmark_gap_size.sh $occ 200 1000 1 $file ${prefix}DC0_read1_gap_200pages_${occ}occ.csv

echo "Test 12/12: File: ${prefix}DC0_read2_gap_200pages_${occ}occ.csv"
./benchmark_gap_size.sh $occ 200 1000 2 $file ${prefix}DC0_read2_gap_200pages_${occ}occ.csv
