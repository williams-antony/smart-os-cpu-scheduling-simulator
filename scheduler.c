/*
 * SMART OS: Interactive Process Scheduling and Performance Analysis Simulator
 * Algorithms: FCFS, SJF (non-preemptive), Priority (non-preemptive), Round Robin
 * Compile: gcc scheduler.c -o scheduler
 * Run:     ./scheduler
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 50

typedef struct {
    char id[10];
    int at, bt, pr;
    int rt, rem, ct, tat, wt, first;
} Process;

typedef struct {
    char id[10];
    int start, end;
} Segment;

void reset(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].rem = p[i].bt;
        p[i].ct = p[i].tat = p[i].wt = 0;
        p[i].rt = -1;
        p[i].first = -1;
    }
}

void sortByArrival(Process p[], int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (p[j].at < p[i].at) {
                Process t = p[i];
                p[i] = p[j];
                p[j] = t;
            }
}

void addSeg(Segment g[], int *gc, const char *id, int s, int e) {
    if (s == e) return;
    if (*gc > 0 && strcmp(g[*gc - 1].id, id) == 0 && g[*gc - 1].end == s)
        g[*gc - 1].end = e;
    else {
        strcpy(g[*gc].id, id);
        g[*gc].start = s;
        g[*gc].end = e;
        (*gc)++;
    }
}

void finish(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
        p[i].rt = p[i].first - p[i].at;
    }
}

void fcfs(Process p[], int n, Segment g[], int *gc) {
    reset(p, n);
    sortByArrival(p, n);
    int time = 0;
    for (int i = 0; i < n; i++) {
        if (time < p[i].at) time = p[i].at;
        p[i].first = time;
        addSeg(g, gc, p[i].id, time, time + p[i].bt);
        time += p[i].bt;
        p[i].ct = time;
    }
    finish(p, n);
}

int chooseSJF(Process p[], int n, int time, int used[]) {
    int idx = -1;
    for (int i = 0; i < n; i++)
        if (!used[i] && p[i].at <= time)
            if (idx == -1 || p[i].bt < p[idx].bt ||
                (p[i].bt == p[idx].bt && p[i].at < p[idx].at))
                idx = i;
    return idx;
}

void sjf(Process p[], int n, Segment g[], int *gc) {
    reset(p, n);
    int used[MAX] = {0}, done = 0, time = 0;
    while (done < n) {
        int k = chooseSJF(p, n, time, used);
        if (k == -1) {
            int next = 1000000;
            for (int i = 0; i < n; i++)
                if (!used[i] && p[i].at < next) next = p[i].at;
            time = next;
            continue;
        }
        used[k] = 1;
        p[k].first = time;
        addSeg(g, gc, p[k].id, time, time + p[k].bt);
        time += p[k].bt;
        p[k].ct = time;
        done++;
    }
    finish(p, n);
}

int choosePriority(Process p[], int n, int time, int used[]) {
    int idx = -1;
    for (int i = 0; i < n; i++)
        if (!used[i] && p[i].at <= time)
            if (idx == -1 || p[i].pr < p[idx].pr ||
                (p[i].pr == p[idx].pr && p[i].at < p[idx].at))
                idx = i;
    return idx;
}

void priority(Process p[], int n, Segment g[], int *gc) {
    reset(p, n);
    int used[MAX] = {0}, done = 0, time = 0;
    while (done < n) {
        int k = choosePriority(p, n, time, used);
        if (k == -1) {
            int next = 1000000;
            for (int i = 0; i < n; i++)
                if (!used[i] && p[i].at < next) next = p[i].at;
            time = next;
            continue;
        }
        used[k] = 1;
        p[k].first = time;
        addSeg(g, gc, p[k].id, time, time + p[k].bt);
        time += p[k].bt;
        p[k].ct = time;
        done++;
    }
    finish(p, n);
}

void roundRobin(Process p[], int n, int q, Segment g[], int *gc) {
    reset(p, n);
    Process a[MAX];
    memcpy(a, p, sizeof(Process) * n);
    sortByArrival(a, n);
    int qv[MAX * 4], front = 0, rear = 0, time = 0, next = 0, done = 0;
    int map[MAX];
    for (int i = 0; i < n; i++) map[i] = i;
    while (done < n) {
        if (front == rear && next < n && time < a[next].at) time = a[next].at;
        while (next < n && a[next].at <= time) qv[rear++] = next++;
        if (front == rear) continue;
        int ai = qv[front++];
        int k = map[ai];
        if (p[k].first == -1) p[k].first = time;
        int run = p[k].rem < q ? p[k].rem : q;
        addSeg(g, gc, p[k].id, time, time + run);
        time += run;
        p[k].rem -= run;
        while (next < n && a[next].at <= time) qv[rear++] = next++;
        if (p[k].rem > 0) qv[rear++] = ai;
        else { p[k].ct = time; done++; }
    }
    finish(p, n);
}

void printResult(Process p[], int n, Segment g[], int gc) {
    double aw = 0, at = 0, ar = 0;
    printf("\nGANTT CHART\n");
    for (int i = 0; i < gc; i++) printf("| %s ", g[i].id);
    printf("|\n%d", g[0].start);
    for (int i = 0; i < gc; i++) printf("\t%d", g[i].end);
    printf("\n\nProcess\tAT\tBT\tPR\tCT\tTAT\tWT\tRT\n");
    for (int i = 0; i < n; i++) {
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               p[i].id, p[i].at, p[i].bt, p[i].pr,
               p[i].ct, p[i].tat, p[i].wt, p[i].rt);
        aw += p[i].wt;
        at += p[i].tat;
        ar += p[i].rt;
    }
    printf("\nAverage Waiting Time    : %.2f\n", aw / n);
    printf("Average Turnaround Time : %.2f\n", at / n);
    printf("Average Response Time   : %.2f\n", ar / n);
    printf("Context Switches        : %d\n", gc > 0 ? gc - 1 : 0);
}

int main() {
    Process p[MAX];
    Segment g[MAX * 4];
    int n, choice, q, gc;

    printf("SMART OS - PROCESS SCHEDULER SIMULATOR\n");
    printf("Enter number of processes: ");
    scanf("%d", &n);
    if (n < 1 || n > MAX) return 1;

    for (int i = 0; i < n; i++) {
        sprintf(p[i].id, "P%d", i + 1);
        printf("\nProcess %s\n", p[i].id);
        printf("Arrival Time : "); scanf("%d", &p[i].at);
        printf("Burst Time   : "); scanf("%d", &p[i].bt);
        printf("Priority     : "); scanf("%d", &p[i].pr);
    }

    printf("\n1. FCFS\n2. SJF\n3. Priority\n4. Round Robin\n");
    printf("Choose algorithm: ");
    scanf("%d", &choice);

    gc = 0;
    if (choice == 1) fcfs(p, n, g, &gc);
    else if (choice == 2) sjf(p, n, g, &gc);
    else if (choice == 3) priority(p, n, g, &gc);
    else if (choice == 4) {
        printf("Time Quantum: ");
        scanf("%d", &q);
        if (q <= 0) return 1;
        roundRobin(p, n, q, g, &gc);
    } else {
        printf("Invalid choice.\n");
        return 1;
    }

    printResult(p, n, g, gc);
    return 0;
}
