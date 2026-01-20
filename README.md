# Process Scheduling

## Overview
Implementation of two advanced process scheduling algorithms for the Xinu operating system that address starvation: an Exponential Distribution Scheduler and a Linux-like Scheduler (emulating Linux 2.2 kernel).

## Key Features
- **Exponential Distribution Scheduler (EXPDISTSCHED)**: Probabilistic scheduler using exponential distribution (λ=0.1) to prevent starvation while favoring higher-priority processes
- **Linux-like Scheduler (LINUXSCHED)**: Epoch-based scheduler with dynamic goodness calculation, quantum allocation, and carryover mechanism
- Dynamic scheduling policy selection via `setschedclass()` and `getschedclass()`
- Fair scheduling for processes with equal priorities

## Files Modified/Created

### Created Files
- [`h/sched.h`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/h/sched.h) - Scheduler constants and function declarations
- [`sys/expdist.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/expdist.c) - Exponential distribution scheduler implementation
- [`sys/lnxsched.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/lnxsched.c) - Linux-like scheduler implementation
- [`sys/sched.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/sched.c) - Scheduler class management functions

### Modified Files
- [`h/proc.h`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/h/proc.h) - Added Linux scheduler fields (lnx_counter, lnx_goodness, lnx_pexhaust, lnx_new)
- [`sys/resched.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/resched.c) - Updated to dispatch to appropriate scheduler
- [`sys/create.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/create.c) - Initialize scheduler-specific fields for new processes
- [`sys/ready.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/ready.c) - Updated for scheduler integration
- [`sys/chprio.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/chprio.c) - Handle priority changes with Linux scheduler epoch considerations
- [`sys/initialize.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/initialize.c) - Initialize scheduler subsystem
- [`sys/insert.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/insert.c) - Modified queue insertion logic for scheduler requirements
- [`compile/Makefile`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/compile/Makefile) - Added new source files to build

## How to Build and Run

### Build
```bash
cd compile
make
```

### Run
The system boots into Xinu and runs the test program which prompts for scheduler selection:
- Enter `0` or `1` for Exponential Distribution Scheduler test
- Enter `2` or higher for Linux-like Scheduler test

### Clean
```bash
cd compile
make clean
```

## Expected Results

### Exponential Distribution Scheduler
- Process execution ratio approximately **0.63:0.23:0.14** for priorities 10:20:30
- Example output: `Test Result: A = 982590, B = 370179, C = 206867`

### Linux-like Scheduler
- Character output shows epoch-based scheduling with highest priority process running first
- Approximate character count ratio reflects priority ratios within each epoch
- Example pattern: `MCCCCCCCCCCCCCBBBBBBBMMM...`

## Notes/Assumptions
- NULL process (priority 0) only runs when no other processes are ready
- Exponential scheduler uses `expdev(0.1)` from [`sys/math.c`](https://github.com/devangsaraogi/xinu-scheduler/blob/main/sys/math.c)
- Linux scheduler processes created mid-epoch wait until next epoch
- Priority changes take effect at next epoch boundary for Linux scheduler
- Test programs should not be placed in `main.c` as it will be replaced during grading
- All debugging output must be disabled before submission

## Acknowledgments

This project was completed as part of the graduate-level Operating Systems course (Fall 2025), **CSC 501: Operating Systems Principles**, at North Carolina State University, instructed by **Prof. Man-Ki Yoon**.

## Additional Resources
- See [ASSIGNMENT.md](ASSIGNMENT.md) for complete project specification
- Test cases included in assignment document
