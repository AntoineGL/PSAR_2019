#!/bin/bash
#benchmark_moyenne.sh

#Ce script va appeler $2 fois le programme benchmark avec les arguments
#Va faire la moyenne des temps

if [ "$EUID" -ne 0 ];then
  echo "You must be root to clear page cache."
  exit 1
fi

if [ $# -ne 6 ];then
    echo "Usage $0 <type (sequential | random | gap=<gap size>)"
    echo "         <How many occurence> "
    echo "         <How many pages to write> "
    echo "         <Preread> "
    echo "         <File to write> "
    echo "         <output>"
    exit 1
fi

if [[ $2 -le 0 ]];then
    echo "number of occurence must be >0"
    exit 1
fi

total_time=0
ret=0
gap=`sed -rn 's/^gap=([0-9]+)$/\1/gp' <<< $1`

if [ -z "$gap" ];then
    gap=0
fi

declare -a tab_values

for i in `seq 1 $2`;do
    ret=`./benchmark -m $1 -p $3 -r $4 $5`
    
    if [[ $? -ne 0 ]];then
	echo "An error has occured, $0 aborted"
	exit 2
    fi

    tab_values+=("$ret")
    total_time=`echo $total_time + $ret | bc -l`
done

average=`echo $total_time / $2 | bc -l`

variance=0

for j in "${tab_values[@]}";do
    variance=`echo "$variance + (($j - $average)^2)" | bc -l`
done

export LC_NUMERIC=C

variance=`echo "$variance / $2" | bc -l`
std_deviation=`echo "sqrt($variance)" | bc -l`

printf "$3,$gap,%0.6f,%0.6f,$2\n" $average $std_deviation | tee -a $6







