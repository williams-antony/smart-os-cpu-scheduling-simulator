/*
 * Smart OS - CPU Scheduler, Banker's Algorithm and Page Replacement simulator
 * Console version in C (same logic as the web simulator).
 *
 * Build:  gcc -Wall -O2 smart_os.c -o smart_os
 * Run:    ./smart_os
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAXP   50      /* max processes (scheduling) */
#define MAXSEG 60000   /* max Gantt segments */
#define MP     10      /* max processes (Banker's) */
#define MR     5       /* max resource types (Banker's) */
#define MAXREF 60      /* max reference string length */
#define MAXF   10      /* max frames */
#define PAGES  1000    /* page numbers 0..999 */
#define EPS    1e-9

/* ---------- small input helper ---------- */
static int readInt(const char *msg, int lo, int hi) {
    int v;
    for (;;) {
        printf("%s", msg);
        if (scanf("%d", &v) != 1) {
            if (feof(stdin)) exit(0);
            if (scanf("%*s") < 0) exit(0);
            printf("  Invalid number.\n");
            continue;
        }
        if (v < lo || v > hi) { printf("  Enter a value between %d and %d.\n", lo, hi); continue; }
        return v;
    }
}

/* =====================================================================
 *  1. CPU SCHEDULING
 * ===================================================================== */
typedef struct { int id, at, bt, pr, rem, ct, tat, wt, rt, first; } Proc;
typedef struct { double wt, tat, rt; int cs; } Result;

static const char *ALG[4] = { "FCFS", "SJF", "Priority", "Round Robin" };
static Proc procs[MAXP];
static int np = 0;

static int gid[MAXSEG], gs[MAXSEG], ge[MAXSEG], gn;

static void addSeg(int id, int s, int e) {
    if (s == e) return;
    if (gn && gid[gn - 1] == id && ge[gn - 1] == s) { ge[gn - 1] = e; return; }
    gid[gn] = id; gs[gn] = s; ge[gn] = e; gn++;
}

static void sortByArrival(Proc *p, int n) {          /* stable insertion sort */
    for (int i = 1; i < n; i++) {
        Proc x = p[i]; int j = i - 1;
        while (j >= 0 && p[j].at > x.at) { p[j + 1] = p[j]; j--; }
        p[j + 1] = x;
    }
}

static void sortById(Proc *p, int n) {
    for (int i = 1; i < n; i++) {
        Proc x = p[i]; int j = i - 1;
        while (j >= 0 && p[j].id > x.id) { p[j + 1] = p[j]; j--; }
        p[j + 1] = x;
    }
}

static void loadSampleProcs(void) {
    int s[4][3] = { {0,5,2}, {1,3,1}, {2,8,3}, {3,2,2} };
    np = 4;
    for (int i = 0; i < np; i++) {
        procs[i].id = i + 1; procs[i].at = s[i][0]; procs[i].bt = s[i][1]; procs[i].pr = s[i][2];
    }
}

static void enterProcs(void) {
    np = readInt("Number of processes (1-50): ", 1, MAXP);
    for (int i = 0; i < np; i++) {
        printf("P%d\n", i + 1);
        procs[i].id = i + 1;
        procs[i].at = readInt("  Arrival time (0-1000): ", 0, 1000);
        procs[i].bt = readInt("  Burst time   (1-1000): ", 1, 1000);
        procs[i].pr = readInt("  Priority (lower = higher priority): ", 0, 1000);
    }
}

static void printGantt(void) {
    printf("\nGantt chart:\n ");
    for (int i = 0; i < gn; i++) printf("|  P%-2d ", gid[i]);
    printf("|\n ");
    for (int i = 0; i < gn; i++) printf("%-6d", gs[i]);
    printf("%d\n", ge[gn - 1]);
}

static Result schedule(int alg, int q, int show) {
    Proc p[MAXP];
    int n = np, t = 0;
    memcpy(p, procs, n * sizeof(Proc));
    gn = 0;
    for (int i = 0; i < n; i++) { p[i].rem = p[i].bt; p[i].ct = 0; p[i].first = -1; }

    if (alg == 0) {                                   /* FCFS */
        sortByArrival(p, n);
        for (int i = 0; i < n; i++) {
            if (t < p[i].at) t = p[i].at;
            p[i].first = t;
            addSeg(p[i].id, t, t + p[i].bt);
            t += p[i].bt; p[i].ct = t;
        }
    } else if (alg == 1 || alg == 2) {                /* SJF / Priority (non-preemptive) */
        int used[MAXP] = {0}, done = 0;
        while (done < n) {
            int k = -1;
            for (int i = 0; i < n; i++) {
                if (used[i] || p[i].at > t) continue;
                if (k < 0) { k = i; continue; }
                int a = alg == 1 ? p[i].bt : p[i].pr, b = alg == 1 ? p[k].bt : p[k].pr;
                if (a < b || (a == b && p[i].at < p[k].at)) k = i;
            }
            if (k < 0) {                              /* CPU idle: jump to next arrival */
                int mn = INT_MAX;
                for (int i = 0; i < n; i++) if (!used[i] && p[i].at < mn) mn = p[i].at;
                t = mn; continue;
            }
            used[k] = 1; p[k].first = t;
            addSeg(p[k].id, t, t + p[k].bt);
            t += p[k].bt; p[k].ct = t; done++;
        }
    } else {                                          /* Round Robin */
        int Q[MAXP + 1], head = 0, tail = 0, cnt = 0, nx = 0, done = 0;
        sortByArrival(p, n);
        while (done < n) {
            if (!cnt && nx < n && t < p[nx].at) t = p[nx].at;
            while (nx < n && p[nx].at <= t) { Q[tail] = nx++; tail = (tail + 1) % (MAXP + 1); cnt++; }
            if (!cnt) continue;
            int x = Q[head]; head = (head + 1) % (MAXP + 1); cnt--;
            if (p[x].first < 0) p[x].first = t;
            int r = p[x].rem < q ? p[x].rem : q;
            addSeg(p[x].id, t, t + r);
            t += r; p[x].rem -= r;
            while (nx < n && p[nx].at <= t) { Q[tail] = nx++; tail = (tail + 1) % (MAXP + 1); cnt++; }
            if (p[x].rem > 0) { Q[tail] = x; tail = (tail + 1) % (MAXP + 1); cnt++; }
            else { p[x].ct = t; done++; }
        }
    }

    Result r = {0, 0, 0, gn > 0 ? gn - 1 : 0};
    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt  = p[i].tat - p[i].bt;
        p[i].rt  = p[i].first - p[i].at;
        r.wt += p[i].wt; r.tat += p[i].tat; r.rt += p[i].rt;
    }
    r.wt /= n; r.tat /= n; r.rt /= n;

    if (show) {
        sortById(p, n);
        printf("\n=== %s%s ===", ALG[alg], alg == 3 ? " (quantum shown below)" : "");
        if (alg == 3) printf("\nTime quantum = %d", q);
        printGantt();
        printf("\n%-8s%5s%5s%5s%6s%6s%6s%6s\n", "Process", "AT", "BT", "PR", "CT", "TAT", "WT", "RT");
        for (int i = 0; i < n; i++)
            printf("P%-7d%5d%5d%5d%6d%6d%6d%6d\n", p[i].id, p[i].at, p[i].bt, p[i].pr,
                   p[i].ct, p[i].tat, p[i].wt, p[i].rt);
        printf("\nAvg waiting     : %.2f\nAvg turnaround  : %.2f\nAvg response    : %.2f\nContext switches: %d\n",
               r.wt, r.tat, r.rt, r.cs);
    }
    return r;
}

static void compareSched(int q) {
    Result r[4]; double best = 1e18, worst = -1;
    for (int i = 0; i < 4; i++) {
        r[i] = schedule(i, q, 0);
        if (r[i].wt < best) best = r[i].wt;
        if (r[i].wt > worst) worst = r[i].wt;
    }
    printf("\n=== Comparison (Round Robin quantum = %d) ===\n", q);
    printf("%-13s%10s%12s%10s%8s\n", "Algorithm", "Avg WT", "Avg TAT", "Avg RT", "CS");
    for (int i = 0; i < 4; i++) {
        const char *tag = r[i].wt - best < EPS ? "  <- BEST" : (worst - best > EPS && worst - r[i].wt < EPS ? "  <- WORST" : "");
        printf("%-13s%10.2f%12.2f%10.2f%8d%s\n", ALG[i], r[i].wt, r[i].tat, r[i].rt, r[i].cs, tag);
    }
    if (worst - best < EPS) printf("All algorithms tie on this workload.\n");
    printf("(Best / worst judged by average waiting time.)\n");
}

static void schedMenu(void) {
    int q = 2;
    if (!np) loadSampleProcs();
    for (;;) {
        printf("\n--- CPU Scheduling ---\n1) Load sample data\n2) Enter processes\n3) Run one algorithm\n4) Compare all algorithms\n0) Back\n");
        int c = readInt("Choice: ", 0, 4);
        if (c == 0) return;
        if (c == 1) { loadSampleProcs(); printf("Sample loaded (%d processes).\n", np); }
        else if (c == 2) enterProcs();
        else if (c == 3) {
            printf("1) FCFS  2) SJF  3) Priority  4) Round Robin\n");
            int a = readInt("Algorithm: ", 1, 4) - 1;
            if (a == 3) q = readInt("Time quantum (1-100): ", 1, 100);
            schedule(a, q, 1);
        } else {
            q = readInt("Time quantum for Round Robin (1-100): ", 1, 100);
            compareSched(q);
        }
    }
}

/* =====================================================================
 *  2. BANKER'S ALGORITHM
 * ===================================================================== */
static int bn = 0, bm = 0;
static int balloc[MP][MR], bmax[MP][MR], bavail[MR];

static void loadSampleBanker(void) {
    int al[5][3] = { {0,1,0}, {2,0,0}, {3,0,2}, {2,1,1}, {0,0,2} };
    int mx[5][3] = { {7,5,3}, {3,2,2}, {9,0,2}, {2,2,2}, {4,3,3} };
    int av[3] = { 3, 3, 2 };
    bn = 5; bm = 3;
    for (int i = 0; i < bn; i++) for (int j = 0; j < bm; j++) { balloc[i][j] = al[i][j]; bmax[i][j] = mx[i][j]; }
    for (int j = 0; j < bm; j++) bavail[j] = av[j];
}

static void enterBanker(void) {
    bn = readInt("Number of processes (1-10): ", 1, MP);
    bm = readInt("Number of resource types (1-5): ", 1, MR);
    for (int i = 0; i < bn; i++) {
        printf("P%d allocation (%d values): ", i, bm);
        for (int j = 0; j < bm; j++) balloc[i][j] = readInt("", 0, 1000);
    }
    for (int i = 0; i < bn; i++) {
        printf("P%d max demand (%d values): ", i, bm);
        for (int j = 0; j < bm; j++) bmax[i][j] = readInt("", 0, 1000);
    }
    printf("Available (%d values): ", bm);
    for (int j = 0; j < bm; j++) bavail[j] = readInt("", 0, 1000);
}

static void printBankerState(void) {
    printf("\n%-5s| %-*s| %-*s| %-*s\n", "Proc", bm * 3 + 1, "Alloc", bm * 3 + 1, "Max", bm * 3 + 1, "Need");
    for (int i = 0; i < bn; i++) {
        printf("P%-4d|", i);
        for (int j = 0; j < bm; j++) printf("%3d", balloc[i][j]);
        printf(" |");
        for (int j = 0; j < bm; j++) printf("%3d", bmax[i][j]);
        printf(" |");
        for (int j = 0; j < bm; j++) printf("%3d", bmax[i][j] - balloc[i][j]);
        printf("\n");
    }
    printf("Available:");
    for (int j = 0; j < bm; j++) printf("%3d", bavail[j]);
    printf("\n");
}

/* returns 1 if safe; fills seq[]; prints trace when verbose */
static int safety(int al[][MR], int av[], int seq[], int verbose) {
    int work[MR], fin[MP] = {0}, cnt = 0, progress = 1;
    for (int j = 0; j < bm; j++) work[j] = av[j];
    if (verbose) printf("\nSafety trace:\n");
    while (progress) {
        progress = 0;
        for (int i = 0; i < bn; i++) {
            if (fin[i]) continue;
            int ok = 1;
            for (int j = 0; j < bm; j++) if (bmax[i][j] - al[i][j] > work[j]) { ok = 0; break; }
            if (!ok) continue;
            if (verbose) { printf("  P%d runs. Work:", i); for (int j = 0; j < bm; j++) printf(" %d", work[j]); }
            for (int j = 0; j < bm; j++) work[j] += al[i][j];
            if (verbose) { printf(" -> "); for (int j = 0; j < bm; j++) printf("%d ", work[j]); printf("\n"); }
            fin[i] = 1; seq[cnt++] = i; progress = 1;
        }
    }
    if (verbose && cnt < bn) {
        printf("  Blocked:");
        for (int i = 0; i < bn; i++) if (!fin[i]) printf(" P%d", i);
        printf("\n");
    }
    return cnt == bn;
}

static void bankerCheck(void) {
    int seq[MP];
    for (int i = 0; i < bn; i++) for (int j = 0; j < bm; j++)
        if (balloc[i][j] > bmax[i][j]) { printf("Invalid state: P%d allocation exceeds its max.\n", i); return; }
    printBankerState();
    if (safety(balloc, bavail, seq, 1)) {
        printf("\nSAFE state. Safe sequence: ");
        for (int i = 0; i < bn; i++) printf("P%d%s", seq[i], i < bn - 1 ? " -> " : "\n");
    } else printf("\nUNSAFE state. No safe sequence exists, deadlock is possible.\n");
}

static void bankerRequest(void) {
    int seq[MP], req[MR], tal[MP][MR], tav[MR];
    int p = readInt("Requesting process id: ", 0, bn - 1);
    printf("Request (%d values): ", bm);
    for (int j = 0; j < bm; j++) req[j] = readInt("", 0, 1000);
    for (int j = 0; j < bm; j++)
        if (req[j] > bmax[p][j] - balloc[p][j]) { printf("Denied: request exceeds the process's maximum claim.\n"); return; }
    for (int j = 0; j < bm; j++)
        if (req[j] > bavail[j]) { printf("Denied: resources not available, P%d must wait.\n", p); return; }
    memcpy(tal, balloc, sizeof(balloc));
    for (int j = 0; j < bm; j++) { tav[j] = bavail[j] - req[j]; tal[p][j] += req[j]; }
    if (!safety(tal, tav, seq, 0)) { printf("Denied: granting it would leave the system UNSAFE.\n"); return; }
    memcpy(balloc, tal, sizeof(balloc));
    for (int j = 0; j < bm; j++) bavail[j] = tav[j];
    printf("Granted. State stays safe. Safe sequence: ");
    for (int i = 0; i < bn; i++) printf("P%d%s", seq[i], i < bn - 1 ? " -> " : "\n");
}

static void bankerMenu(void) {
    if (!bn) loadSampleBanker();
    for (;;) {
        printf("\n--- Banker's Algorithm ---\n1) Load sample state\n2) Enter custom state\n3) Show state and safety check\n4) Resource request\n0) Back\n");
        int c = readInt("Choice: ", 0, 4);
        if (c == 0) return;
        if (c == 1) { loadSampleBanker(); printf("Sample loaded.\n"); }
        else if (c == 2) enterBanker();
        else if (c == 3) bankerCheck();
        else bankerRequest();
    }
}

/* =====================================================================
 *  3. PAGE REPLACEMENT
 * ===================================================================== */
static const char *PALG[4] = { "FIFO", "LRU", "Optimal", "LFU" };
static int refs[MAXREF] = { 7,0,1,2,0,3,0,4,2,3,0,3,2,1,2,0,1,7,0,1 };
static int nref = 20, nfr = 3;

static int pager(int alg, int show) {
    int fr[MAXF], cnt = 0, flt = 0;
    int ld[PAGES] = {0}, lu[PAGES] = {0}, fq[PAGES] = {0};
    int snap[MAXREF][MAXF], fpos[MAXREF];

    for (int i = 0; i < nref; i++) {
        int p = refs[i], pos = -1;
        for (int j = 0; j < cnt; j++) if (fr[j] == p) pos = j;
        if (pos >= 0) { lu[p] = i; fq[p]++; fpos[i] = -1; }
        else {
            flt++;
            if (cnt < nfr) { pos = cnt; fr[cnt++] = p; }
            else {
                int v = 0;
                for (int j = 1; j < cnt; j++) {
                    int a = fr[j], b = fr[v];
                    if (alg == 0 && ld[a] < ld[b]) v = j;
                    else if (alg == 1 && lu[a] < lu[b]) v = j;
                    else if (alg == 2) {
                        int na = INT_MAX, nb = INT_MAX;
                        for (int k = i + 1; k < nref; k++) { if (refs[k] == a) { na = k; break; } }
                        for (int k = i + 1; k < nref; k++) { if (refs[k] == b) { nb = k; break; } }
                        if (na > nb) v = j;
                    } else if (alg == 3 && (fq[a] < fq[b] || (fq[a] == fq[b] && ld[a] < ld[b]))) v = j;
                }
                fr[v] = p; pos = v;
            }
            ld[p] = i; lu[p] = i; fq[p] = 1;
            fpos[i] = pos;
        }
        for (int j = 0; j < nfr; j++) snap[i][j] = j < cnt ? fr[j] : -1;
    }

    if (show) {
        printf("\n=== %s (%d frames) ===\nRef  ", PALG[alg], nfr);
        for (int i = 0; i < nref; i++) printf("%3d ", refs[i]);
        for (int j = 0; j < nfr; j++) {
            printf("\nF%-4d", j + 1);
            for (int i = 0; i < nref; i++) {
                if (snap[i][j] < 0) printf("  . ");
                else printf("%2d%c ", snap[i][j], fpos[i] == j ? '*' : ' ');
            }
        }
        printf("\n     ");
        for (int i = 0; i < nref; i++) printf("  %c ", fpos[i] < 0 ? 'H' : 'F');
        printf("\n(* = page just loaded, H = hit, F = fault)\n");
        printf("\nPage faults : %d\nPage hits   : %d\nHit ratio   : %.1f%%\nFault ratio : %.1f%%\n",
               flt, nref - flt, 100.0 * (nref - flt) / nref, 100.0 * flt / nref);
    }
    return flt;
}

static void comparePages(void) {
    int f[4], best = INT_MAX, worst = -1;
    for (int i = 0; i < 4; i++) {
        f[i] = pager(i, 0);
        if (f[i] < best) best = f[i];
        if (f[i] > worst) worst = f[i];
    }
    printf("\n=== Comparison (%d frames, %d references) ===\n%-10s%8s%12s\n", nfr, nref, "Algorithm", "Faults", "Hit ratio");
    for (int i = 0; i < 4; i++)
        printf("%-10s%8d%11.1f%%%s\n", PALG[i], f[i], 100.0 * (nref - f[i]) / nref,
               f[i] == best ? "  <- BEST" : (worst > best && f[i] == worst ? "  <- WORST" : ""));
    if (worst == best) printf("All algorithms tie.\n");
    printf("(Best / worst judged by fewest page faults.)\n");
}

static void enterRefs(void) {
    nref = readInt("Reference string length (1-60): ", 1, MAXREF);
    printf("Enter %d page numbers (0-999): ", nref);
    for (int i = 0; i < nref; i++) refs[i] = readInt("", 0, PAGES - 1);
}

static void pageMenu(void) {
    for (;;) {
        printf("\n--- Page Replacement ---\n1) Enter reference string (current length %d)\n2) Set frames (current %d)\n3) Run one algorithm\n4) Compare all algorithms\n0) Back\n", nref, nfr);
        int c = readInt("Choice: ", 0, 4);
        if (c == 0) return;
        if (c == 1) enterRefs();
        else if (c == 2) nfr = readInt("Number of frames (1-10): ", 1, MAXF);
        else if (c == 3) {
            printf("1) FIFO  2) LRU  3) Optimal  4) LFU\n");
            pager(readInt("Algorithm: ", 1, 4) - 1, 1);
        } else comparePages();
    }
}

/* =====================================================================
 *  MAIN
 * ===================================================================== */
int main(void) {
    printf("=====================================\n   Smart OS - Algorithm Simulator\n=====================================\n");
    for (;;) {
        printf("\n1) CPU Scheduling\n2) Banker's Algorithm\n3) Page Replacement\n0) Exit\n");
        int c = readInt("Choice: ", 0, 3);
        if (c == 0) { printf("Goodbye!\n"); return 0; }
        if (c == 1) schedMenu();
        else if (c == 2) bankerMenu();
        else pageMenu();
    }
}
