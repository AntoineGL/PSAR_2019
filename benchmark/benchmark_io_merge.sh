#!/bin/bash
#benchmark_io_merge.sh

if [ $# -lt 5 ];then
    echo "Usage $0 <disk (ex: sda)>"
    echo "         <partition (ex: sda1)>"
    echo "         <type (sequential | gap=X)"
    echo "         <file name (on the partition)>"
    echo "         <out file>"
    exit 1
fi

disk=$1
partition=$2
type="$3"
file=$4
out=$5

io_req_bef=0
io_merged_bef=0
io_req_aft=0
io_merged_aft=0
stat=""

declare -a nb=(1 2 3 4 8 12 16 24 32 48 64 80 96 128 192 256 384 512 768 1024 1536 2048 3072 4096 5120 6144 7168 8192 10144 12192 14240 16384 18432 20480 22528 24576 26624 28672 30720 32768 34816 36864 38912 40960 43008 45056 47104 49152 51200 53248 55296 57344 59392 61440 63488 65536 67584 69632 71680 73728 75776 77824 79872)
export LC_NUMERIC=C
for nb_pages in ${nb[@]};do
    stat=`cat /sys/block/$1/$2/stat | tr -s ' '`
    io_req_bef=`echo -n $stat | cut -d ' ' -f 5`
    io_merged_bef=`echo -n $stat | cut -d ' ' -f 6`
    ./benchmark -m $type -p $nb_pages $file > /dev/null
    if [[ $? -ne 0 ]];then
	echo "An error has occured, $0 aborted"
	exit 2
    fi
    stat=`cat /sys/block/$1/$2/stat | tr -s ' '`
    io_req_aft=`echo -n $stat | cut -d ' ' -f 5`
    io_merged_aft=`echo -n $stat | cut -d ' ' -f 6`
    io_req_aft=`echo "${io_req_aft}-${io_req_bef}" | bc -l`
    io_merged_aft=`echo "${io_merged_aft}-${io_merged_bef}" | bc -l`

    rapport=`echo "$io_merged_aft/$io_req_aft" | bc -l` #nombre de requêtes qui ont été mergées en 1 requete disque en moyenne
    printf "$nb_pages,$type,$io_req_aft,$io_merged_aft,%0.3f\n" $rapport | tee -a $out
    
done
