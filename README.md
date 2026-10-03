# Smart OS: Interactive Process Scheduling and Performance Analysis Simulator

An interactive CPU scheduling simulator that shows how an operating system decides which process runs next. Enter a workload, pick an algorithm, and watch the Gantt chart, ready queue and performance metrics.

Mini-project for **21CSC202J - Operating Systems**, SRM Institute of Science and Technology, Ramapuram Campus, Chennai.


## What is in this repository

| File | Purpose |
|---|---|
| `index.html` | The web simulator (single file, no build step, no backend) |
| `scheduler.c` | Console version in C, as given in the report appendix |
| `Smart_OS_FINAL.pdf` | Full project report |

## Algorithms

| Algorithm | Type | Rule |
|---|---|---|
| FCFS | Non-preemptive | Run in arrival order |
| SJF | Non-preemptive | Shortest burst among arrived processes |
| Priority | Non-preemptive | Lowest priority number first |
| Round Robin | Preemptive | Fixed time quantum, cyclic order |

Ties go to the earlier arrival, then the lower process index.

## Web simulator features

- Editable process table (arrival, burst, priority), up to 50 processes
- Animated Gantt chart with step-by-step playback, speed control and a time slider
- Live view of the running process, ready queue and finished processes
- Per-process CT, TAT, WT, RT plus averages, context switches, CPU utilization and throughput
- Configurable time quantum, context-switch cost and priority aging
- Side-by-side comparison of all six algorithms with a best-algorithm summary
- Quantum sweep chart for Round Robin
- "Why was this process picked?" decision log; click an entry to jump to that moment
- Light and dark themes, responsive layout

## Run the web version

Open `index.html` in any modern browser. Nothing to install.

To host it: **Settings → Pages → Deploy from branch → `main` / root**.

## Build and run the C version

```bash
gcc scheduler.c -o scheduler
./scheduler
```

The program asks for the number of processes, then arrival time, burst time and priority for each, then the algorithm (and a time quantum for Round Robin). It prints a text Gantt chart and the metrics table.

## Formulas

```
Turnaround Time (TAT) = Completion Time (CT) - Arrival Time (AT)
Waiting Time (WT)     = TAT - Burst Time (BT)
Response Time (RT)    = First Start Time - AT
```

## Sample results

Workload: P1 (AT 0, BT 5, PR 2), P2 (1, 3, 1), P3 (2, 8, 3), P4 (3, 2, 2). Round Robin quantum = 2.

| Algorithm | Avg WT | Avg TAT | Avg RT | Context switches |
|---|---|---|---|---|
| FCFS | 5.75 | 10.25 | 5.75 | 3 |
| SJF | 4.00 | 8.50 | 4.00 | 3 |
| Priority | 4.25 | 8.75 | 4.25 | 3 |
| Round Robin (q=2) | 7.25 | 11.75 | 2.00 | 8 |

Context switches count transitions between distinct execution segments; switch time is not charged unless a context-switch cost is set in the web version.

## Limitations

- Models scheduling behaviour only; it does not replace a real OS scheduler.
- The C version covers FCFS, SJF, Priority and Round Robin (non-preemptive SJF and Priority). SRTF, preemptive Priority and the analysis tools are in the web version.
- Input is assumed to be valid numeric data.

## Future work

Multilevel Queue and Multilevel Feedback Queue scheduling, random workload batch benchmarking, and CPU idle-time analysis.

## References

1. Silberschatz, Galvin, Gagne - *Operating System Concepts*, Wiley
2. Tanenbaum, Bos - *Modern Operating Systems*, Pearson
3. Stallings - *Operating Systems: Internals and Design Principles*, Pearson
