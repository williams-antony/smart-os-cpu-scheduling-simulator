# Smart OS · CPU Scheduler Simulator

An interactive, single-file web app that visualizes core operating system algorithms: **CPU scheduling**, **deadlock avoidance (Banker's Algorithm)** and **page replacement**. No build step, no dependencies. Just open the HTML file in a browser.

## Features

### 1. CPU Scheduling
- Algorithms: **FCFS**, **SJF** (non-preemptive), **Priority** (non-preemptive), **Round Robin** (adjustable time quantum)
- Editable process table (arrival time, burst time, priority), up to 50 processes
- Sample data and random workload generator
- Animated **Gantt chart** with a time ruler
- Metrics: average waiting time, average turnaround time, average response time, context switches
- Per-process table: CT, TAT, WT, RT
- **Compare all algorithms** on the same workload, with the **best (★)** and **worst (▼)** algorithm marked by average waiting time

### 2. Banker's Algorithm
- Editable Allocation, Max and Available matrices (resources A, B, C; up to 10 processes)
- Computes the Need matrix (`Need = Max − Allocation`)
- Safety check: reports **SAFE** or **UNSAFE** state, the **safe sequence**, and a step-by-step trace of the Work vector
- **Resource request** simulator: a request is granted only if it is within Need, within Available, and leaves the system in a safe state

### 3. Page Replacement
- Algorithms: **FIFO**, **LRU**, **Optimal**, **LFU**
- Custom reference string (up to 60 pages) and 1 to 10 frames
- Frame-by-frame table with hit and fault highlighting
- Stats: page faults, hits, hit ratio, fault ratio
- **Compare all algorithms**, with the **best (★)** and **worst (▼)** marked by fewest page faults

### Extras
- Light and dark theme (follows system setting, with a manual toggle)
- Responsive layout for desktop and mobile
- Respects `prefers-reduced-motion`

## Getting Started

1. Download `smart-os-simulator.html`.
2. Open it in any modern browser (Chrome, Edge, Firefox, Safari).

Fonts load from Google Fonts. Without internet access the app still works and falls back to system fonts.

### Host on GitHub Pages
1. Rename the file to `index.html`.
2. Go to **Settings → Pages**, choose your branch and the `/ (root)` folder, then save.
3. Your simulator will be live at `https://<your-username>.github.io/<repo-name>/`.

## How It Works

### Scheduling formulas
| Metric | Formula |
|---|---|
| Turnaround Time (TAT) | Completion Time − Arrival Time |
| Waiting Time (WT) | TAT − Burst Time |
| Response Time (RT) | First CPU time − Arrival Time |

- **FCFS**: runs processes in arrival order.
- **SJF**: among arrived processes, picks the smallest burst; ties go to the earlier arrival.
- **Priority**: picks the lowest priority number (highest priority) among arrived processes.
- **Round Robin**: each process runs for up to one quantum, then goes to the back of the ready queue.

### Banker's Algorithm
1. Compute `Need = Max − Allocation`.
2. Start with `Work = Available`.
3. Find an unfinished process whose Need ≤ Work, run it, and add its Allocation to Work.
4. Repeat. If every process finishes, the state is safe and the finishing order is a safe sequence.

### Page replacement policies
| Policy | Evicts |
|---|---|
| FIFO | The page that has been in memory the longest |
| LRU | The page not used for the longest time |
| Optimal | The page whose next use is farthest in the future (benchmark, needs future knowledge) |
| LFU | The page referenced the fewest times; ties go to the earliest loaded |

## Tech Stack
- HTML, CSS and vanilla JavaScript (single file)
- No frameworks, no build tools


