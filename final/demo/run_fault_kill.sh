#!/bin/bash

PORT=${PORT:-2120}
SIZE=${SIZE:-32}
WORKERS=${WORKERS:-4}
GRANULARITY=${GRANULARITY:-8}
RUN_ID=${RUN_ID:-50}
KILLS=${KILLS:-1}
KILL_DELAY=${KILL_DELAY:-1}

CSV=fault_timings.csv
MASTER_LOG=fault_master.log
WORKER_LOG=fault_workers.log

echo "mode,run_id,matrix_size,workers,granularity,kills_requested,kills_done,killed_pids,elapsed_seconds,status" > "$CSV"
rm -f "$MASTER_LOG" "$WORKER_LOG"

cleanup()
{
    pkill -f "./worker $PORT $RUN_ID" 2>/dev/null
    pkill -f "./master $PORT" 2>/dev/null
    pkill -f "./tsh $PORT" 2>/dev/null
}

cleanup
sleep 1

cd ~/public_html/final/tsh || exit 1
./tsh "$PORT" > /dev/null 2>&1 &
TSH_PID=$!
sleep 1

cd ~/public_html/final/demo || exit 1
for ((i = 0; i < WORKERS; i++))
do
    WORKER_DELAY_SECONDS=3 ./worker "$PORT" "$RUN_ID" >> "$WORKER_LOG" 2>&1 &
done

sleep 1

./master "$PORT" "$SIZE" "$WORKERS" "$GRANULARITY" "$RUN_ID" > "$MASTER_LOG" 2>&1 &
MASTER_PID=$!

KILLED_PIDS=""
KILLS_DONE=0

for ((i = 0; i < KILLS; i++))
do
    sleep "$KILL_DELAY"
    VICTIM=$(pgrep -f "./worker $PORT $RUN_ID" | shuf -n 1)

    if [ -n "$VICTIM" ]; then
        echo "Killing worker pid $VICTIM" | tee -a "$WORKER_LOG"
        kill "$VICTIM" 2>/dev/null
        KILLED_PIDS="${KILLED_PIDS}${VICTIM};"
        KILLS_DONE=$((KILLS_DONE + 1))
    else
        echo "No worker available to kill" | tee -a "$WORKER_LOG"
    fi
done

wait "$MASTER_PID"
MASTER_STATUS=$?

ELAPSED=$(awk '/Elapsed time/ {print $3}' "$MASTER_LOG")
if [ "$MASTER_STATUS" -eq 0 ] && [ -n "$ELAPSED" ]; then
    STATUS=completed
else
    STATUS=failed
    ELAPSED=${ELAPSED:-NA}
fi

echo "random_kill,$RUN_ID,$SIZE,$WORKERS,$GRANULARITY,$KILLS,$KILLS_DONE,$KILLED_PIDS,$ELAPSED,$STATUS" >> "$CSV"

echo "Fault-kill test complete."
echo "Killed workers: ${KILLED_PIDS:-none}"
echo "Master status: $STATUS"
echo "Elapsed seconds: $ELAPSED"
echo
echo "Master log:"
cat "$MASTER_LOG"
echo
echo "Worker log:"
cat "$WORKER_LOG"

cleanup
wait "$TSH_PID" 2>/dev/null
