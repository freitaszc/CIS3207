#!/bin/bash

PORT=${PORT:-2110}
SIZE=${SIZE:-32}
WORKERS=${WORKERS:-4}

echo "granularity,elapsed_seconds" > timings.csv

for G in 1 2 4 8 16
do
    RUN_PORT=$((PORT + G))
    RUN_ID=$G

    pkill -f "./tsh $RUN_PORT" 2>/dev/null
    pkill -f "./worker $RUN_PORT $RUN_ID" 2>/dev/null

    cd ~/public_html/final/tsh
    ./tsh $RUN_PORT > /dev/null 2>&1 &
    sleep 1

    cd ~/public_html/final/demo
    for ((i=0; i<WORKERS; i++))
    do
        ./worker $RUN_PORT $RUN_ID > /dev/null 2>&1 &
    done

    sleep 1

    OUT=$(timeout 30s ./master $RUN_PORT $SIZE $WORKERS $G $RUN_ID)
    TIME=$(echo "$OUT" | awk '/Elapsed time/ {print $3}')
    TIME=${TIME:-NA}

    echo "$G,$TIME" >> timings.csv

    pkill -f "./worker $RUN_PORT $RUN_ID" 2>/dev/null
    pkill -f "./tsh $RUN_PORT" 2>/dev/null
    sleep 1
done

echo "Done. Results saved to timings.csv"
