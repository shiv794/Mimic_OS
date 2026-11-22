#include "SimulationEngine.h"
#include <algorithm>
#include <queue>
#include <sstream>
#include <iomanip>

using namespace std;

static string escapeJsonString(const string& s) {
    ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

static string intArrayToJson(const vector<int>& vec) {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        oss << vec[i];
        if (i + 1 < vec.size()) oss << ",";
    }
    oss << "]";
    return oss.str();
}

string SimulationResult::toJson() const {
    ostringstream oss;
    oss << fixed << setprecision(2);
    oss << "{\n";
    oss << "  \"algorithm\": \"" << escapeJsonString(algorithm) << "\",\n";
    oss << "  \"timeQuantum\": " << timeQuantum << ",\n";
    oss << "  \"totalTime\": " << totalTime << ",\n";
    oss << "  \"idleTime\": " << idleTime << ",\n";
    oss << "  \"avgTurnaroundTime\": " << avgTurnaroundTime << ",\n";
    oss << "  \"avgWaitingTime\": " << avgWaitingTime << ",\n";
    oss << "  \"avgResponseTime\": " << avgResponseTime << ",\n";
    oss << "  \"cpuUtilization\": " << cpuUtilization << ",\n";
    oss << "  \"throughput\": " << throughput << ",\n";

    // Gantt chart items
    oss << "  \"gantt\": [\n";
    for (size_t i = 0; i < gantt.size(); i++) {
        oss << "    {\"pid\": " << gantt[i].pid
            << ", \"name\": \"" << escapeJsonString(gantt[i].name) << "\""
            << ", \"startTime\": " << gantt[i].startTime
            << ", \"endTime\": " << gantt[i].endTime << "}";
        if (i + 1 < gantt.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    // Process statistics
    oss << "  \"stats\": [\n";
    for (size_t i = 0; i < stats.size(); i++) {
        oss << "    {\"pid\": " << stats[i].pid
            << ", \"name\": \"" << escapeJsonString(stats[i].name) << "\""
            << ", \"arrivalTime\": " << stats[i].arrivalTime
            << ", \"burstTime\": " << stats[i].burstTime
            << ", \"priority\": " << stats[i].priority
            << ", \"startTime\": " << stats[i].startTime
            << ", \"completionTime\": " << stats[i].completionTime
            << ", \"turnaroundTime\": " << stats[i].turnaroundTime
            << ", \"waitingTime\": " << stats[i].waitingTime
            << ", \"responseTime\": " << stats[i].responseTime << "}";
        if (i + 1 < stats.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    // Chronological Steps
    oss << "  \"steps\": [\n";
    for (size_t i = 0; i < steps.size(); i++) {
        oss << "    {\"stepNumber\": " << steps[i].stepNumber
            << ", \"time\": " << steps[i].time
            << ", \"eventType\": \"" << escapeJsonString(steps[i].eventType) << "\""
            << ", \"description\": \"" << escapeJsonString(steps[i].description) << "\""
            << ", \"runningPid\": " << steps[i].runningPid
            << ", \"runningName\": \"" << escapeJsonString(steps[i].runningName) << "\""
            << ", \"remainingBurst\": " << steps[i].remainingBurst
            << ", \"readyQueue\": " << intArrayToJson(steps[i].readyQueue)
            << ", \"completed\": " << intArrayToJson(steps[i].completed) << "}";
        if (i + 1 < steps.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ]\n";

    oss << "}";
    return oss.str();
}

static void calculateAverages(SimulationResult& res) {
    if (res.stats.empty()) {
        res.avgTurnaroundTime = 0;
        res.avgWaitingTime = 0;
        res.avgResponseTime = 0;
        res.cpuUtilization = 0;
        res.throughput = 0;
        return;
    }

    double sumTAT = 0, sumWT = 0, sumRT = 0;
    for (const auto& s : res.stats) {
        sumTAT += s.turnaroundTime;
        sumWT += s.waitingTime;
        sumRT += s.responseTime;
    }
    int n = res.stats.size();
    res.avgTurnaroundTime = sumTAT / n;
    res.avgWaitingTime = sumWT / n;
    res.avgResponseTime = sumRT / n;

    if (res.totalTime > 0) {
        int busyTime = res.totalTime - res.idleTime;
        res.cpuUtilization = (double)busyTime * 100.0 / res.totalTime;
        res.throughput = (double)n / res.totalTime;
    } else {
        res.cpuUtilization = 0;
        res.throughput = 0;
    }
}

// ================= FCFS =================
SimulationResult SimulationEngine::runFCFS(const vector<Process>& processes) {
    SimulationResult res;
    res.algorithm = "FCFS";
    res.timeQuantum = 0;
    res.idleTime = 0;

    if (processes.empty()) {
        res.totalTime = 0;
        calculateAverages(res);
        return res;
    }

    vector<Process> procList = processes;
    sort(procList.begin(), procList.end(), [](const Process& a, const Process& b) {
        if (a.getArrivalTime() != b.getArrivalTime())
            return a.getArrivalTime() < b.getArrivalTime();
        return a.getPID() < b.getPID();
    });

    int currentTime = 0;
    int stepCounter = 1;
    vector<int> completedPids;

    for (size_t i = 0; i < procList.size(); i++) {
        const Process& p = procList[i];

        // Handle CPU Idle if process has not arrived yet
        if (currentTime < p.getArrivalTime()) {
            int idleDuration = p.getArrivalTime() - currentTime;
            res.idleTime += idleDuration;

            StepRecord idleStep;
            idleStep.stepNumber = stepCounter++;
            idleStep.time = currentTime;
            idleStep.eventType = "idle";
            idleStep.description = "Time " + to_string(currentTime) + " -> CPU is IDLE waiting for processes to arrive.";
            idleStep.runningPid = -1;
            idleStep.runningName = "IDLE";
            idleStep.remainingBurst = 0;
            idleStep.readyQueue = {};
            idleStep.completed = completedPids;
            res.steps.push_back(idleStep);

            // Record idle in Gantt chart if needed
            GanttItem idleItem;
            idleItem.pid = -1;
            idleItem.name = "IDLE";
            idleItem.startTime = currentTime;
            idleItem.endTime = p.getArrivalTime();
            res.gantt.push_back(idleItem);

            currentTime = p.getArrivalTime();
        }

        // Ready queue at dispatch time
        vector<int> readyQ;
        for (size_t j = i; j < procList.size(); j++) {
            if (procList[j].getArrivalTime() <= currentTime) {
                readyQ.push_back(procList[j].getPID());
            }
        }

        // Dispatch step
        StepRecord dispatchStep;
        dispatchStep.stepNumber = stepCounter++;
        dispatchStep.time = currentTime;
        dispatchStep.eventType = "dispatch";
        dispatchStep.description = "Time " + to_string(currentTime) + " -> CPU selects " + p.getName() + " (PID: " + to_string(p.getPID()) + ")";
        dispatchStep.runningPid = p.getPID();
        dispatchStep.runningName = p.getName();
        dispatchStep.remainingBurst = p.getBurstTime();
        dispatchStep.readyQueue = readyQ;
        dispatchStep.completed = completedPids;
        res.steps.push_back(dispatchStep);

        int startTime = currentTime;
        int endTime = currentTime + p.getBurstTime();

        GanttItem item;
        item.pid = p.getPID();
        item.name = p.getName();
        item.startTime = startTime;
        item.endTime = endTime;
        res.gantt.push_back(item);

        currentTime = endTime;
        completedPids.push_back(p.getPID());

        // Completed step
        StepRecord compStep;
        compStep.stepNumber = stepCounter++;
        compStep.time = currentTime;
        compStep.eventType = "complete";
        compStep.description = "Time " + to_string(currentTime) + " -> " + p.getName() + " completed execution.";
        compStep.runningPid = -1;
        compStep.runningName = "None";
        compStep.remainingBurst = 0;
        compStep.readyQueue = {};
        compStep.completed = completedPids;
        res.steps.push_back(compStep);

        ProcessStat stat;
        stat.pid = p.getPID();
        stat.name = p.getName();
        stat.arrivalTime = p.getArrivalTime();
        stat.burstTime = p.getBurstTime();
        stat.priority = p.getPriority();
        stat.startTime = startTime;
        stat.completionTime = endTime;
        stat.turnaroundTime = stat.completionTime - stat.arrivalTime;
        stat.waitingTime = stat.turnaroundTime - stat.burstTime;
        stat.responseTime = stat.startTime - stat.arrivalTime;
        res.stats.push_back(stat);
    }

    res.totalTime = currentTime;
    calculateAverages(res);
    return res;
}

// ================= SJF (Non-Preemptive) =================
SimulationResult SimulationEngine::runSJF(const vector<Process>& processes) {
    SimulationResult res;
    res.algorithm = "SJF";
    res.timeQuantum = 0;
    res.idleTime = 0;

    int n = processes.size();
    if (n == 0) {
        res.totalTime = 0;
        calculateAverages(res);
        return res;
    }

    vector<bool> completed(n, false);
    vector<int> completedPids;
    int completedCount = 0;
    int currentTime = 0;
    int stepCounter = 1;

    while (completedCount < n) {
        int selected = -1;
        vector<int> readyQ;

        for (int i = 0; i < n; i++) {
            if (!completed[i] && processes[i].getArrivalTime() <= currentTime) {
                readyQ.push_back(processes[i].getPID());
                if (selected == -1 || processes[i].getBurstTime() < processes[selected].getBurstTime()) {
                    selected = i;
                } else if (processes[i].getBurstTime() == processes[selected].getBurstTime()) {
                    if (processes[i].getArrivalTime() < processes[selected].getArrivalTime()) {
                        selected = i;
                    }
                }
            }
        }

        if (selected == -1) {
            // CPU is idle
            res.idleTime++;
            StepRecord idleStep;
            idleStep.stepNumber = stepCounter++;
            idleStep.time = currentTime;
            idleStep.eventType = "idle";
            idleStep.description = "Time " + to_string(currentTime) + " -> CPU IDLE. Waiting for processes to arrive.";
            idleStep.runningPid = -1;
            idleStep.runningName = "IDLE";
            idleStep.remainingBurst = 0;
            idleStep.readyQueue = {};
            idleStep.completed = completedPids;
            res.steps.push_back(idleStep);

            if (!res.gantt.empty() && res.gantt.back().pid == -1) {
                res.gantt.back().endTime++;
            } else {
                GanttItem idleItem;
                idleItem.pid = -1;
                idleItem.name = "IDLE";
                idleItem.startTime = currentTime;
                idleItem.endTime = currentTime + 1;
                res.gantt.push_back(idleItem);
            }

            currentTime++;
            continue;
        }

        const Process& p = processes[selected];

        // Dispatch step
        StepRecord dispatchStep;
        dispatchStep.stepNumber = stepCounter++;
        dispatchStep.time = currentTime;
        dispatchStep.eventType = "dispatch";
        dispatchStep.description = "Time " + to_string(currentTime) + " -> SJF selects " + p.getName() + " (Shortest Burst: " + to_string(p.getBurstTime()) + " units)";
        dispatchStep.runningPid = p.getPID();
        dispatchStep.runningName = p.getName();
        dispatchStep.remainingBurst = p.getBurstTime();
        dispatchStep.readyQueue = readyQ;
        dispatchStep.completed = completedPids;
        res.steps.push_back(dispatchStep);

        int startTime = currentTime;
        int endTime = currentTime + p.getBurstTime();

        GanttItem item;
        item.pid = p.getPID();
        item.name = p.getName();
        item.startTime = startTime;
        item.endTime = endTime;
        res.gantt.push_back(item);

        currentTime = endTime;
        completed[selected] = true;
        completedCount++;
        completedPids.push_back(p.getPID());

        // Completion step
        StepRecord compStep;
        compStep.stepNumber = stepCounter++;
        compStep.time = currentTime;
        compStep.eventType = "complete";
        compStep.description = "Time " + to_string(currentTime) + " -> " + p.getName() + " completed.";
        compStep.runningPid = -1;
        compStep.runningName = "None";
        compStep.remainingBurst = 0;
        compStep.readyQueue = {};
        compStep.completed = completedPids;
        res.steps.push_back(compStep);

        ProcessStat stat;
        stat.pid = p.getPID();
        stat.name = p.getName();
        stat.arrivalTime = p.getArrivalTime();
        stat.burstTime = p.getBurstTime();
        stat.priority = p.getPriority();
        stat.startTime = startTime;
        stat.completionTime = endTime;
        stat.turnaroundTime = stat.completionTime - stat.arrivalTime;
        stat.waitingTime = stat.turnaroundTime - stat.burstTime;
        stat.responseTime = stat.startTime - stat.arrivalTime;
        res.stats.push_back(stat);
    }

    res.totalTime = currentTime;
    calculateAverages(res);
    return res;
}

// ================= Priority Scheduling (Non-Preemptive) =================
SimulationResult SimulationEngine::runPriority(const vector<Process>& processes) {
    SimulationResult res;
    res.algorithm = "Priority";
    res.timeQuantum = 0;
    res.idleTime = 0;

    int n = processes.size();
    if (n == 0) {
        res.totalTime = 0;
        calculateAverages(res);
        return res;
    }

    vector<bool> completed(n, false);
    vector<int> completedPids;
    int completedCount = 0;
    int currentTime = 0;
    int stepCounter = 1;

    while (completedCount < n) {
        int selected = -1;
        vector<int> readyQ;

        for (int i = 0; i < n; i++) {
            if (!completed[i] && processes[i].getArrivalTime() <= currentTime) {
                readyQ.push_back(processes[i].getPID());
                // Lower priority number = higher priority
                if (selected == -1 || processes[i].getPriority() < processes[selected].getPriority()) {
                    selected = i;
                } else if (processes[i].getPriority() == processes[selected].getPriority()) {
                    if (processes[i].getArrivalTime() < processes[selected].getArrivalTime()) {
                        selected = i;
                    }
                }
            }
        }

        if (selected == -1) {
            res.idleTime++;
            StepRecord idleStep;
            idleStep.stepNumber = stepCounter++;
            idleStep.time = currentTime;
            idleStep.eventType = "idle";
            idleStep.description = "Time " + to_string(currentTime) + " -> CPU IDLE. Waiting for processes.";
            idleStep.runningPid = -1;
            idleStep.runningName = "IDLE";
            idleStep.remainingBurst = 0;
            idleStep.readyQueue = {};
            idleStep.completed = completedPids;
            res.steps.push_back(idleStep);

            if (!res.gantt.empty() && res.gantt.back().pid == -1) {
                res.gantt.back().endTime++;
            } else {
                GanttItem idleItem;
                idleItem.pid = -1;
                idleItem.name = "IDLE";
                idleItem.startTime = currentTime;
                idleItem.endTime = currentTime + 1;
                res.gantt.push_back(idleItem);
            }

            currentTime++;
            continue;
        }

        const Process& p = processes[selected];

        StepRecord dispatchStep;
        dispatchStep.stepNumber = stepCounter++;
        dispatchStep.time = currentTime;
        dispatchStep.eventType = "dispatch";
        dispatchStep.description = "Time " + to_string(currentTime) + " -> Priority selects " + p.getName() + " (Priority: " + to_string(p.getPriority()) + ", lower number = higher priority)";
        dispatchStep.runningPid = p.getPID();
        dispatchStep.runningName = p.getName();
        dispatchStep.remainingBurst = p.getBurstTime();
        dispatchStep.readyQueue = readyQ;
        dispatchStep.completed = completedPids;
        res.steps.push_back(dispatchStep);

        int startTime = currentTime;
        int endTime = currentTime + p.getBurstTime();

        GanttItem item;
        item.pid = p.getPID();
        item.name = p.getName();
        item.startTime = startTime;
        item.endTime = endTime;
        res.gantt.push_back(item);

        currentTime = endTime;
        completed[selected] = true;
        completedCount++;
        completedPids.push_back(p.getPID());

        StepRecord compStep;
        compStep.stepNumber = stepCounter++;
        compStep.time = currentTime;
        compStep.eventType = "complete";
        compStep.description = "Time " + to_string(currentTime) + " -> " + p.getName() + " completed.";
        compStep.runningPid = -1;
        compStep.runningName = "None";
        compStep.remainingBurst = 0;
        compStep.readyQueue = {};
        compStep.completed = completedPids;
        res.steps.push_back(compStep);

        ProcessStat stat;
        stat.pid = p.getPID();
        stat.name = p.getName();
        stat.arrivalTime = p.getArrivalTime();
        stat.burstTime = p.getBurstTime();
        stat.priority = p.getPriority();
        stat.startTime = startTime;
        stat.completionTime = endTime;
        stat.turnaroundTime = stat.completionTime - stat.arrivalTime;
        stat.waitingTime = stat.turnaroundTime - stat.burstTime;
        stat.responseTime = stat.startTime - stat.arrivalTime;
        res.stats.push_back(stat);
    }

    res.totalTime = currentTime;
    calculateAverages(res);
    return res;
}

// ================= Round Robin (Preemptive) =================
SimulationResult SimulationEngine::runRoundRobin(const vector<Process>& processes, int timeQuantum) {
    SimulationResult res;
    res.algorithm = "Round Robin";
    res.timeQuantum = max(1, timeQuantum);
    res.idleTime = 0;

    int n = processes.size();
    if (n == 0) {
        res.totalTime = 0;
        calculateAverages(res);
        return res;
    }

    vector<int> remainingTime(n);
    vector<int> firstStartTime(n, -1);
    vector<int> completionTime(n, 0);
    vector<bool> enqueued(n, false);
    vector<int> completedPids;
    queue<int> readyQ;

    for (int i = 0; i < n; i++) {
        remainingTime[i] = processes[i].getBurstTime();
    }

    int currentTime = 0;
    int completedCount = 0;
    int stepCounter = 1;

    while (completedCount < n) {
        // Enqueue processes arrived at currentTime
        for (int i = 0; i < n; i++) {
            if (!enqueued[i] && processes[i].getArrivalTime() <= currentTime) {
                readyQ.push(i);
                enqueued[i] = true;
            }
        }

        if (readyQ.empty()) {
            res.idleTime++;
            StepRecord idleStep;
            idleStep.stepNumber = stepCounter++;
            idleStep.time = currentTime;
            idleStep.eventType = "idle";
            idleStep.description = "Time " + to_string(currentTime) + " -> CPU IDLE. Ready queue is empty.";
            idleStep.runningPid = -1;
            idleStep.runningName = "IDLE";
            idleStep.remainingBurst = 0;
            idleStep.readyQueue = {};
            idleStep.completed = completedPids;
            res.steps.push_back(idleStep);

            if (!res.gantt.empty() && res.gantt.back().pid == -1) {
                res.gantt.back().endTime++;
            } else {
                GanttItem idleItem;
                idleItem.pid = -1;
                idleItem.name = "IDLE";
                idleItem.startTime = currentTime;
                idleItem.endTime = currentTime + 1;
                res.gantt.push_back(idleItem);
            }

            currentTime++;
            continue;
        }

        int index = readyQ.front();
        readyQ.pop();

        const Process& p = processes[index];
        if (firstStartTime[index] == -1) {
            firstStartTime[index] = currentTime;
        }

        // Snapshot of current ready queue
        vector<int> readyQSnapshot;
        queue<int> tempQ = readyQ;
        while (!tempQ.empty()) {
            readyQSnapshot.push_back(processes[tempQ.front()].getPID());
            tempQ.pop();
        }

        int execDuration = min(res.timeQuantum, remainingTime[index]);

        StepRecord dispatchStep;
        dispatchStep.stepNumber = stepCounter++;
        dispatchStep.time = currentTime;
        dispatchStep.eventType = "dispatch";
        dispatchStep.description = "Time " + to_string(currentTime) + " -> Dispatched " + p.getName() + " for time quantum " + to_string(execDuration) + " units (Remaining burst: " + to_string(remainingTime[index]) + ")";
        dispatchStep.runningPid = p.getPID();
        dispatchStep.runningName = p.getName();
        dispatchStep.remainingBurst = remainingTime[index];
        dispatchStep.readyQueue = readyQSnapshot;
        dispatchStep.completed = completedPids;
        res.steps.push_back(dispatchStep);

        int startTime = currentTime;
        currentTime += execDuration;
        remainingTime[index] -= execDuration;

        GanttItem item;
        item.pid = p.getPID();
        item.name = p.getName();
        item.startTime = startTime;
        item.endTime = currentTime;
        res.gantt.push_back(item);

        // Check for new arrivals during the time slice
        for (int i = 0; i < n; i++) {
            if (!enqueued[i] && processes[i].getArrivalTime() <= currentTime) {
                readyQ.push(i);
                enqueued[i] = true;
            }
        }

        if (remainingTime[index] == 0) {
            completedCount++;
            completionTime[index] = currentTime;
            completedPids.push_back(p.getPID());

            StepRecord compStep;
            compStep.stepNumber = stepCounter++;
            compStep.time = currentTime;
            compStep.eventType = "complete";
            compStep.description = "Time " + to_string(currentTime) + " -> " + p.getName() + " finished execution and terminated.";
            compStep.runningPid = -1;
            compStep.runningName = "None";
            compStep.remainingBurst = 0;
            compStep.readyQueue = readyQSnapshot;
            compStep.completed = completedPids;
            res.steps.push_back(compStep);
        } else {
            // Re-enqueue (Circular queue)
            readyQ.push(index);

            StepRecord preemptStep;
            preemptStep.stepNumber = stepCounter++;
            preemptStep.time = currentTime;
            preemptStep.eventType = "preempt";
            preemptStep.description = "Time " + to_string(currentTime) + " -> Quantum expired for " + p.getName() + ". Remaining: " + to_string(remainingTime[index]) + " units. Rotated to back of ready queue.";
            preemptStep.runningPid = -1;
            preemptStep.runningName = "None";
            preemptStep.remainingBurst = remainingTime[index];
            preemptStep.readyQueue = readyQSnapshot;
            preemptStep.completed = completedPids;
            res.steps.push_back(preemptStep);
        }
    }

    res.totalTime = currentTime;

    for (int i = 0; i < n; i++) {
        const Process& p = processes[i];
        ProcessStat stat;
        stat.pid = p.getPID();
        stat.name = p.getName();
        stat.arrivalTime = p.getArrivalTime();
        stat.burstTime = p.getBurstTime();
        stat.priority = p.getPriority();
        stat.startTime = firstStartTime[i];
        stat.completionTime = completionTime[i];
        stat.turnaroundTime = stat.completionTime - stat.arrivalTime;
        stat.waitingTime = stat.turnaroundTime - stat.burstTime;
        stat.responseTime = stat.startTime - stat.arrivalTime;
        res.stats.push_back(stat);
    }

    calculateAverages(res);
    return res;
}

