# Final Project – Tuple Space & Parallel Matrix Multiplication

## Overview
This project extends the Tuple Space Handler (TSH) by building an API library and a parallel matrix multiplication application using a master/worker model.

The system uses a single tuple space, where all communication between processes happens through tuples.

---

## Question 1 – API Library

I finalized the API library (`tshlib.c/h`) so applications can interact directly with TSH.

### Implemented Functions
- `tsh_put()` inserts a tuple into the tuple space
- `tsh_get()` removes a tuple from the tuple space
- `tsh_read()` reads a tuple without removing it
- `tsh_exit_cmd()` terminates TSH
- `tsh_shell_cmd()` executes shell commands through TSH

### Key Improvements
- Removed the need for `tsInit()` by connecting inside each API call
- Fixed pointer handling using `void **tuple`
- Allocated memory dynamically for returned tuples
- Ensured correct network byte order conversion (`htonl`, `ntohl`, etc.)
- Converted `tshtest` from an interactive menu into a non-interactive
  regression test with expected PASS/FAIL results
- Verified `OP_SHELL` through the API test path instead of only manual input

### OP_SHELL Service

`OP_SHELL` is implemented as a TSH service and is also exposed through
`tsh_shell_cmd()` and `launch`. The server now uses the Lab 4-style shell
parser/executor path for normal commands, including argv execution, output
redirection, and pipelines. Commands ending with `&` are forked as background
processes so `launch` can start workers in parallel.

Regression coverage:

    cd final/tsh
    ./tsh 2110 &
    ./tshtest 2110

Expected result:

    PASS: tsh_put stores tuple
    PASS: tsh_read returns expected tuple without removing it
    PASS: tsh_get returns expected tuple
    PASS: tsh_get removes tuple from tuple space
    PASS: OP_SHELL executes foreground command
    PASS: OP_SHELL returns expected foreground output
    PASS: OP_SHELL executes Lab 4 style pipeline
    PASS: OP_SHELL pipeline returns expected output
    Summary: 0 failure(s)

---

## Question 2 – Parallel Matrix Multiplication

I implemented a parallel matrix multiplication program using a master/worker architecture.

### Architecture

**Master**
- Creates matrices A and B
- Splits matrix rows among workers
- Sends tasks (`TASK_<run_id>_<task_id>`) into tuple space
- Waits for results (`RESULT_<run_id>_<task_id>`)
- Assembles the final matrix C

**Worker**
- Waits for matrices A and B
- Retrieves a task for its assigned run ID
- Computes assigned rows using **ijk-order multiplication**
- Sends results back to tuple space

---

## Tuple Naming

We use unique tuple names so one tuple space can safely hold matrix data,
tasks, results, and termination messages:

- `A_<run_id>` for matrix A
- `B_<run_id>` for matrix B
- `TASK_<run_id>_0`, `TASK_<run_id>_1` for worker tasks
- `RESULT_<run_id>_0`, `RESULT_<run_id>_1` for worker results
- `STOP_<run_id>_0`, `STOP_<run_id>_1` for worker termination

To avoid incorrect matching, exact tuple names are used for single tuples and
run-specific prefixes are used only for task and stop groups:

    ^A_<run_id>$, ^B_<run_id>$, TASK_<run_id>_, ^RESULT_<run_id>_0$, STOP_<run_id>_

---

## Errors Encountered and Fixes

### 1. Segmentation Fault (Worker)
**Cause:**
- Reused the same pointer for multiple `tsh_read()` calls

**Fix:**
- Used separate pointers (`tupleA`, `tupleB`, `tupleTask`)
- Freed allocated memory correctly

### 2. Stack Smashing Error
**Cause:**
- Memory corruption due to incorrect tuple handling

**Fix:**
- Corrected pointer usage
- Ensured proper struct casting

### 3. Incorrect Tuple Matching
**Cause:**
- Broad tuple expressions could match the wrong tuple group.

**Fix:**
- Used anchored expressions for single tuples and simple prefixes for unique
  run-specific tuple groups:

    ^A_RUNID$, ^B_RUNID$, TASK_RUNID_, STOP_RUNID_

### 4. Workers Crashing When Started First
**Cause:**
- Workers attempted to read data before it existed

**Fix:**
- Added retry loops such as:

    while (!tsh_read(...)) { usleep(...); }

### 5. Master Crashing While Waiting for Results
**Cause:**
- Attempted to use tuple data before confirming retrieval

**Fix:**
- Added a loop to wait until `tsh_get()` succeeds

### 6. Linker Error (`tsh_shell_cmd`)
**Cause:**
- Function was missing after modifying `tshlib.c`

**Fix:**
- Reimplemented `tsh_shell_cmd()`

### 7. `EXPR_LEN` Undefined Error
**Cause:**
- Used an undefined constant

**Fix:**
- Replaced it with:

    sizeof(out.expr)

---

## Key Takeaways

- Tuple naming must be precise
- API calls must be validated before using returned data
- Synchronization is necessary in parallel systems
- Memory management (`malloc`/`free`) is critical in C

---

## Final Status

- API library working
- Parallel matrix multiplication working
- Master/worker synchronization working
- Demo runs successfully with 3 terminals

## Questions

### 1. How will run 5 workers in parallel?
To run 5 workers in parallel, I use `launch.c`, which sends shell commands to TSH. The worker needs the TSH port and the run ID. The `&` at the end of each command runs the worker in the background, so all workers execute in parallel.

Example:
cd final/tsh

./launch 2110 "../demo/worker 2110 1 &"
./launch 2110 "../demo/worker 2110 1 &"
./launch 2110 "../demo/worker 2110 1 &"
./launch 2110 "../demo/worker 2110 1 &"
./launch 2110 "../demo/worker 2110 1 &"

### 2. How to control granularity (G) running master?
Granularity is controlled by the command-line arguments passed to `master`. The master program uses:

    ./master PORT MATRIX_SIZE WORKERS GRANULARITY RUN_ID

### 3. How to get the total number of parallel workers? 
To get the total number of parallel workers, I can create a Bash script that launches the workers in a loop and counts how many workers were started.

Example script:

PORT=$1
WORKERS=$2
COUNT=0

for ((i = 0; i < WORKERS; i++))
do
    ../tsh/launch "$PORT" "../demo/worker $PORT $RUN_ID &"
    COUNT=$((COUNT + 1))
done

echo "Total parallel workers launched: $COUNT"

---

## Extra credit

### 1. Find a way to change parallel task granularity and build a script to run the parallel matrix program automatically with different granularity. Record elapsed times for each size.

I added `demo/run_granularity.sh` to automate this test. The script starts a
fresh TSH instance for each granularity value, launches the worker processes,
runs the master program, extracts the elapsed time from the master output, and
writes the result to `demo/timings.csv`.

The master accepts granularity as a command-line argument:

    ./master PORT MATRIX_SIZE WORKERS GRANULARITY RUN_ID

For the latest recorded run, I tested a 32x32 matrix with 4 workers and G
values 1, 2, 4, 8, and 16.

Recorded results:

    granularity,elapsed_seconds
    1,0.120760
    2,0.108654
    4,0.105646
    8,0.102631
    16,0.203086

---

### 2. Revise the parallel program to allow processors to fail by timeout alarm signal handler (re-issuing a suspected lost tuple) and redundant tuple result elimination.

I added timeout recovery in `demo/master.c` using a `SIGALRM` signal handler.
The signal handler sets a timeout flag, and the main master loop checks for
unfinished tasks whose results have not arrived within
`RESULT_TIMEOUT_SECONDS`.

When a task times out, master reissues the same `TASK_<run_id>_<task_id>` tuple
so another available worker can process it. Each task and result includes both
`run_id` and `task_id`, so master can tell which run and task the result belongs
to.

Redundant result elimination is handled with the `completed[]` array. If a
result arrives for a task that was already completed, master ignores it instead
of adding it to matrix C again.

Example recovery messages:

    Timeout: reissued TASK_50_0 attempt 2
    Timeout: reissued TASK_50_1 attempt 2
    Ignoring duplicate RESULT_0
    Ignoring duplicate RESULT_1

---

### 3. Analyze the elapsed times to find the best performing size. Explain why?

From the latest `demo/timings.csv`, the best-performing granularity was `G=8`.

Results:

    G=1   -> 0.120760 seconds
    G=2   -> 0.108654 seconds
    G=4   -> 0.105646 seconds
    G=8   -> 0.102631 seconds
    G=16  -> 0.203086 seconds

`G=8` performed best because it created four tasks for a 32-row matrix, which
matched the four workers. That gave each worker useful work while keeping tuple
communication overhead low. Smaller values like `G=1` and `G=2` created many
more task/result tuples, increasing TSH communication overhead. `G=16` created
only two tasks, so only two workers could do useful computation while the other
workers waited.

---

### 4. Test fault tolerance by randomly kill worker processes. Record your performance changes.

The script `demo/run_fault_kill.sh` starts TSH, starts workers, starts master,
randomly kills a worker during execution, and records the result in
`demo/fault_timings.csv`.

Latest fault-tolerance result:

    mode,run_id,matrix_size,workers,granularity,kills_requested,kills_done,killed_pids,elapsed_seconds,status
    random_kill,50,32,4,8,1,1,3202432;,9.116739,completed

The master recovered by reissuing suspected lost tasks and ignoring duplicate
results:

    Timeout: reissued TASK_50_0 attempt 2
    Timeout: reissued TASK_50_1 attempt 2
    Timeout: reissued TASK_50_2 attempt 2
    Timeout: reissued TASK_50_3 attempt 2
    Ignoring duplicate RESULT_0
    Ignoring duplicate RESULT_1
    Ignoring duplicate RESULT_2

---

### How to kill random workers?
Use Bash or Python to find worker PIDs and randomly kill one or more of them.

Automated test:

    cd ~/public_html/final/demo
    ./run_fault_kill.sh
    cat fault_timings.csv

Example Bash command:

    pkill -f "./worker 2110 1"

For a random single worker:

    pgrep -f "./worker 2110 1" | shuf -n 1 | xargs kill

### How to Master timeout/retransmit?
Master uses a `SIGALRM` handler as a timeout tick. The handler only sets a flag. The main loop checks unfinished tasks, and if a result has not arrived within `RESULT_TIMEOUT_SECONDS`, master reinserts the matching task tuple.

Each task and result includes a `run_id` and `task_id`. Master only accepts a result when both IDs match the current run. If the same result arrives more than once, master ignores the duplicate because `completed[task_id]` is already set.

Example recovery behavior:

    Timeout: reissued TASK_3110857_0 attempt 2
    Timeout: reissued TASK_3110857_0 attempt 3
    Worker received task 0 rows 0 to 7

### How to automate parallel runs with different G values?
The script `demo/run_granularity.sh` automatically runs the matrix program with several G values and records elapsed time in `demo/timings.csv`.

Latest measured result:

    granularity,elapsed_seconds
    1,0.120760
    2,0.108654
    4,0.105646
    8,0.102631
    16,0.203086

### How to debug?
Useful debugging steps:

- Run TSH, master, and workers in separate terminals.
- Use small matrices first, such as `./master 2110 4 2 1 1`.
- Print task IDs and row ranges in the worker.
- Check tuple names carefully so a broad expression does not consume the wrong run's tuple.
- Check `tsh/tsh_status.log` for background process exit status from `launch`.

### How to collect performance data?
Use `gettimeofday()` in master to measure elapsed compute time and save the result from each run into `timings.csv`. The automation script records one row per granularity value.

### Best performing granularity
From the latest `timings.csv`, `G=8` was fastest for size 32 with 4 workers. Very small G creates many task tuples, which increases tuple-space communication overhead. `G=16` creates only two tasks for a 32-row matrix, so only two workers can do useful computation while the other workers wait. `G=8` creates four tasks, which matches the four workers and still keeps tuple overhead low.

### How to write final project report?
The report should include:

- Requirement checklist and what was completed.
- API library explanation, especially why `void **tuple` is needed for returned tuple memory.
- Master/worker design and tuple naming.
- Build and run instructions.
- Screenshots from `screenshots/`.
- Performance table from `demo/timings.csv`.
- Successes, failures, and limitations.

## Screenshot Evidence

Final screenshots:

- [TSH build](screenshots/final_build_tsh.png): shows `make clean` and `make all` building `tsh`, `tshtest`, and `launch`.
- [Demo build](screenshots/final_build_demo.png): should show `make clean` and `make all` building `master`, `worker`, and `api_test`.
- [Run-ID workers](screenshots/final_workers_runid.png): shows four workers launched with the final command format `./worker 2110 1 &`; workers receive tasks and exit.
- [Master result](screenshots/final_master_runid.png): shows `./master 2110 4 4 1 1` completing a 4x4 matrix multiplication with run ID 1.
- [Granularity automation](screenshots/final_granularity.png): shows `./run_granularity.sh`, `cat timings.csv`, and recorded elapsed times for G values 1, 2, 4, 8, and 16.
- [Timeout recovery](screenshots/final_timeout_recovery.png): shows master reissuing `TASK_2_0`, a worker with run ID 2 processing the reissued task, and master completing.
- [Random worker kill fault test](screenshots/final_fault_kill.png): shows `./run_fault_kill.sh`, the killed worker PID, master completion, `fault_timings.csv`, `fault_master.log`, and duplicate-result elimination.

Earlier development screenshots are also included for history:

- [Original TSH launch](screenshots/tsh.png)
- [Original worker launch](screenshots/worker.png)
- [Original worker completion](screenshots/worker_done.png)
- [Original master result](screenshots/master.png)
- [Original three-terminal demo](screenshots/terminals.png)
