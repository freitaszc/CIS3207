# Professor Feedback Change Report

## Overview

After reviewing the professor feedback, the latest updates focused on completing the Lab 6 shell service, making it reusable outside the interactive tester, documenting the work, and preparing the final package for submission. The main result is that TSH now supports a new `OP_SHELL` request, `tshtest` can test it interactively, and the new `launch` client can run shell commands from the command line through TSH.

## Changes Made

### 1. Added the new TSH shell operation

The tuple space operation list was updated so the shell service is part of the normal TSH request system.

Files updated:
- `include/synergy.h`
- `tsh/tsh.h`
- `tsh/tsh.c`

Details:
- Added `TSH_OP_SHELL` as operation code `405`.
- Updated `TSH_OP_MAX` to `405` and `TSH_OP_CNT` to `5`.
- Added `tsh_shell_it` for sending a shell command to the server.
- Added `tsh_shell_ot` for returning status, error, and output length.
- Added the `OpShell()` prototype in `tsh.h`.
- Added `OpShell` to the TSH dispatch table so the server calls it when operation `405` is received.

### 2. Implemented server-side shell command execution

The server now receives a shell command, executes it, and returns the command output to the client.

File updated:
- `tsh/tsh.c`

Details:
- Implemented `OpShell()`.
- Reads a `tsh_shell_it` request from the socket.
- Executes the command using `popen()`.
- Reads the command output into a buffer.
- Sends a structured response containing success/failure status, error code, and output length.
- Sends the shell output back to the client.
- Returns `[no output]` when the command succeeds but produces no output.

### 3. Updated the interactive tester

The `tshtest` program was updated so `OP_SHELL` can be tested from the existing menu.

Files updated:
- `tsh/tshtest.c`
- `tsh/tshtest.h`

Details:
- Added `OpShell()` to the client-side dispatch table.
- Added a new menu option: `5. Shell`.
- Added the `OpShell()` client function.
- Prompts the user for a shell command.
- Sends the command to TSH.
- Reads the response header and command output.
- Null-terminates the received output before printing it.

### 4. Created a reusable TSH shell library

The shell client logic was separated from `tshtest` so other programs can use the shell operation without the interactive menu.

Files added/updated:
- `tsh/tshlib.c`
- `tsh/tshlib.h`

Details:
- Added `connectTshLib()` to connect to TSH on localhost using a provided port.
- Added `tsh_shell_cmd()` to send a shell command and return the output.
- Handles invalid arguments, socket failures, write failures, read failures, and TSH failure responses.
- Ensures returned output is null-terminated.
- Removes dependence on the `tshtest` menu and prompt flow.

### 5. Added the command-line launch client

A new command-line client was added so shell commands can be executed through TSH without using the interactive tester.

Files added/updated:
- `tsh/launch.c`
- `tsh/launch.h`

Details:
- `launch` accepts a port number and a shell command.
- Combines all command arguments into one command string.
- Calls `tsh_shell_cmd()` from the reusable library.
- Prints the returned command output.
- Prints usage help when the required arguments are missing.
- Prints `OP_SHELL failed` when the shell request cannot complete.

Example format:

```bash
./launch 2110 ls
./launch 2110 echo hello
```

### 6. Updated the build system

The Makefile now builds and installs the new `launch` executable with the rest of the TSH programs.

File updated:
- `tsh/makefile`

Details:
- Added `launch` to the `all` target.
- Added a build rule for `launch.c`, `launch.h`, `tshlib.c`, and `tshlib.h`.
- Updated the `copy` target so `launch` is copied into `bin`.
- Updated the `clean` target so `launch` is removed with the other build outputs.

### 7. Updated documentation and evidence

The README and supporting screenshots were updated to show the work completed and the testing process.

Files/directories updated:
- `README.md`
- `screenshots/`
- `docs/html/`
- `Doxyfile`

Details:
- Documented the ping-pong socket test.
- Documented tuple-space `PUT`, `READ`, and `GET` testing.
- Documented the `OP_SHELL` server changes.
- Documented the `tshtest` shell menu test.
- Documented the reusable `tshlib.c` and `tshlib.h` library.
- Documented the `launch` client and command-line testing.
- Added screenshots for the main testing steps, including `op_shell.png`, `op_shell_command.png`, and `launch.png`.
- Generated Doxygen HTML documentation in `docs/html`.

### 8. Updated final binaries and package

The final executable outputs and compressed submission package were updated after the source changes.

Files/directories updated:
- `bin/tsh`
- `bin/tshtest`
- `bin/launch`
- `lab6.tar.gz`

Details:
- Rebuilt the TSH server.
- Rebuilt the interactive `tshtest` client.
- Built the new `launch` client.
- Copied the executables into `bin`.
- Created the final compressed archive for submission.

## Testing Performed

The following tests were documented in the README and screenshots:

- Ran the socket ping-pong example with `lab6` and `lab6c`.
- Started `tsh` on port `2110`.
- Used `tshtest` to test `PUT`, `READ`, and `GET`.
- Confirmed that `GET` removes the tuple after retrieval.
- Used the new `Shell` option in `tshtest` to run a command through TSH.
- Ran `launch` with commands such as `ls` and `echo hello`.
- Confirmed that `launch` fails when TSH is not running and succeeds after the TSH server is started.

## Final Result

The project now includes a complete shell-command service for TSH. The implementation works in both the interactive test client and the new command-line client, uses a reusable library for the shared client logic, builds through the Makefile, includes documentation, and has updated screenshots and packaged submission files.
