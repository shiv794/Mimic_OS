#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include <vector>
#include <string>
#include "Process.h"

using namespace std;

struct GanttItem {
    int pid;
    string name;
    int startTime;
    int endTime;
};

struct StepRecord {
    int stepNumber;
    int time;
    string eventType; // "arrival", "dispatch", "tick", "preempt", "complete", "idle"
    string description;
    int runningPid;
    string runningName;
    int remainingBurst;
    vector<int> readyQueue;
    vector<int> completed;
};

struct ProcessStat {
    int pid;
    string name;
    int arrivalTime;
    int burstTime;
    int priority;
    int startTime;
    int completionTime;
    int turnaroundTime;
    int waitingTime;
    int responseTime;
};

struct SimulationResult {
    string algorithm;
    int timeQuantum;
    vector<GanttItem> gantt;
    vector<StepRecord> steps;
    vector<ProcessStat> stats;
    double avgTurnaroundTime;
    double avgWaitingTime;
    double avgResponseTime;
    double cpuUtilization;
    double throughput;
    int totalTime;
    int idleTime;

    string toJson() const;
};

class SimulationEngine {
public:
    static SimulationResult runFCFS(const vector<Process>& processes);
    static SimulationResult runSJF(const vector<Process>& processes);
    static SimulationResult runPriority(const vector<Process>& processes);
    static SimulationResult runRoundRobin(const vector<Process>& processes, int timeQuantum);
};

#endif

