#include "DeadlockManager.h"
#include <iostream>

using namespace std;

// CONSTRUCTOR

DeadlockManager::DeadlockManager() {
    processCount = 3;
    resourceCount = 3;
    processIds = {1, 2, 3};
    processNames = {"P1", "P2", "P3"};
    loadSafeScenario();
}
// INITIALIZE

void DeadlockManager::initialize(int processes, int resources) {
    processCount = processes;
    resourceCount = resources;

    processIds.resize(processes);
    processNames.resize(processes);
    for (int i = 0; i < processes; i++) {
        processIds[i] = i + 1;
        processNames[i] = "P" + to_string(i + 1);
    }

    allocation.assign(processes, vector<int>(resources, 0));
    request.assign(processes, vector<int>(resources, 0));
    available.assign(resources, 0);
}

// PROCESS IDENTIFIER HELPERS

int DeadlockManager::findProcessIndex(int pid) const {
    for (size_t i = 0; i < processIds.size(); i++) {
        if (processIds[i] == pid) return (int)i;
    }
    return -1;
}

int DeadlockManager::getPidAt(int index) const {
    if (index >= 0 && index < (int)processIds.size()) {
        return processIds[index];
    }
    return index + 1;
}

string DeadlockManager::getNameAt(int index) const {
    if (index >= 0 && index < (int)processNames.size()) {
        return processNames[index];
    }
    return "P" + to_string(getPidAt(index));
}

// PROCESS SYNC & DYNAMIC MANAGEMENT

void DeadlockManager::addProcess(int pid, const string& name) {
    int idx = findProcessIndex(pid);
    if (idx != -1) {
        if (!name.empty()) processNames[idx] = name;
        return;
    }

    if (resourceCount <= 0) resourceCount = 3;
    if (available.size() != (size_t)resourceCount) {
        available.assign(resourceCount, 0);
    }

    processIds.push_back(pid);
    processNames.push_back(name.empty() ? ("P" + to_string(pid)) : name);
    allocation.push_back(vector<int>(resourceCount, 0));
    request.push_back(vector<int>(resourceCount, 0));
    processCount++;
}

bool DeadlockManager::removeProcess(int pid) {
    int idx = findProcessIndex(pid);
    if (idx == -1) return false;

    // Return any allocated resources back to available pool
    for (int j = 0; j < resourceCount; j++) {
        if (j < (int)allocation[idx].size() && j < (int)available.size()) {
            available[j] += allocation[idx][j];
        }
    }

    processIds.erase(processIds.begin() + idx);
    processNames.erase(processNames.begin() + idx);
    allocation.erase(allocation.begin() + idx);
    request.erase(request.begin() + idx);
    processCount--;
    return true;
}

void DeadlockManager::syncWithProcesses(const vector<Process>& procs) {
    if (resourceCount <= 0) resourceCount = 3;
    if (available.size() != (size_t)resourceCount) {
        available.assign(resourceCount, 0);
    }

    // 1. Remove rows for processes that no longer exist
    for (int i = (int)processIds.size() - 1; i >= 0; i--) {
        int pid = processIds[i];
        bool found = false;
        for (const auto& p : procs) {
            if (p.getPID() == pid) {
                found = true;
                break;
            }
        }
        if (!found) {
            for (int j = 0; j < resourceCount; j++) {
                if (j < (int)allocation[i].size() && j < (int)available.size()) {
                    available[j] += allocation[i][j];
                }
            }
            processIds.erase(processIds.begin() + i);
            processNames.erase(processNames.begin() + i);
            allocation.erase(allocation.begin() + i);
            request.erase(request.begin() + i);
            processCount--;
        }
    }

    // 2. Add or update rows for active processes
    for (const auto& p : procs) {
        int pid = p.getPID();
        int idx = findProcessIndex(pid);
        if (idx == -1) {
            processIds.push_back(pid);
            processNames.push_back(p.getName());
            allocation.push_back(vector<int>(resourceCount, 0));
            request.push_back(vector<int>(resourceCount, 0));
            processCount++;
        } else {
            processNames[idx] = p.getName();
        }
    }
}

// SET ALLOCATION / REQUEST / AVAILABLE

void DeadlockManager::setAllocation(int process, int resource, int value) {
    if (process < 0 || process >= processCount || resource < 0 || resource >= resourceCount)
        return;
    allocation[process][resource] = value;
}

void DeadlockManager::setRequest(int process, int resource, int value) {
    if (process < 0 || process >= processCount || resource < 0 || resource >= resourceCount)
        return;
    request[process][resource] = value;
}

void DeadlockManager::setAvailable(int resource, int value) {
    if (resource < 0 || resource >= resourceCount)
        return;
    available[resource] = value;
}

// DISPLAY

void DeadlockManager::display() {
    cout << "\n========== DEADLOCK SYSTEM ==========\n";
    cout << "\nAvailable Resources:\n";
    for (int i = 0; i < resourceCount; i++) {
        cout << "R" << i << ": " << available[i] << endl;
    }

    cout << "\nAllocation Matrix:\n";
    for (int i = 0; i < processCount; i++) {
        string name = (i < (int)processNames.size()) ? processNames[i] : ("P" + to_string(i + 1));
        cout << name << ": ";
        for (int j = 0; j < resourceCount; j++) {
            cout << allocation[i][j] << " ";
        }
        cout << endl;
    }

    cout << "\nRequest Matrix:\n";
    for (int i = 0; i < processCount; i++) {
        string name = (i < (int)processNames.size()) ? processNames[i] : ("P" + to_string(i + 1));
        cout << name << ": ";
        for (int j = 0; j < resourceCount; j++) {
            cout << request[i][j] << " ";
        }
        cout << endl;
    }
    cout << "=====================================\n";
}

// EVALUATE DEADLOCK (Banker's / Cycle Algorithm)

DeadlockResult DeadlockManager::evaluateDeadlock() const {
    DeadlockResult result;
    result.isDeadlocked = false;

    if (processCount == 0 || resourceCount == 0) {
        return result;
    }

    vector<int> work = available;
    vector<bool> finish(processCount, false);
    bool progress = true;

    while (progress) {
        progress = false;

        for (int i = 0; i < processCount; i++) {
            if (finish[i]) continue;

            bool canFinish = true;
            for (int j = 0; j < resourceCount; j++) {
                if (request[i][j] > work[j]) {
                    canFinish = false;
                    break;
                }
            }

            if (canFinish) {
                for (int j = 0; j < resourceCount; j++) {
                    work[j] += allocation[i][j];
                }
                finish[i] = true;
                result.safeSequence.push_back(i);
                result.safeSequencePids.push_back(getPidAt(i));
                result.safeSequenceNames.push_back(getNameAt(i));
                progress = true;
            }
        }
    }

    for (int i = 0; i < processCount; i++) {
        if (!finish[i]) {
            result.isDeadlocked = true;
            result.deadlockedProcesses.push_back(i);
            result.deadlockedPids.push_back(getPidAt(i));
        }
    }

    return result;
}

// DEADLOCK DETECTION (Console Output)

bool DeadlockManager::detectDeadlock() {
    DeadlockResult res = evaluateDeadlock();

    cout << "\n========== DEADLOCK CHECK ==========\n";
    if (res.isDeadlocked) {
        for (int p : res.deadlockedProcesses) {
            cout << getNameAt(p) << " (PID " << getPidAt(p) << ") is involved in deadlock.\n";
        }
    } else {
        cout << "No deadlock detected.\nSafe sequence: ";
        for (size_t i = 0; i < res.safeSequenceNames.size(); i++) {
            cout << res.safeSequenceNames[i] << (i + 1 < res.safeSequenceNames.size() ? " -> " : "\n");
        }
    }
    cout << "\n";

    return res.isDeadlocked;
}
// SCENARIOS

void DeadlockManager::loadSafeScenario() {
    resourceCount = 3;
    if (processCount <= 0) {
        processIds = {1, 2, 3};
        processNames = {"P1", "P2", "P3"};
        processCount = 3;
    }

    allocation.assign(processCount, vector<int>(resourceCount, 0));
    request.assign(processCount, vector<int>(resourceCount, 0));
    available.assign(resourceCount, 0);

    if (processCount == 5) {
        // Classic Silberschatz 5-process textbook scenario
        available = {0, 0, 0};
        allocation[0] = {0, 1, 0};
        allocation[1] = {2, 0, 0};
        allocation[2] = {3, 0, 2};
        allocation[3] = {2, 1, 1};
        allocation[4] = {0, 0, 2};

        request[0] = {0, 0, 0};
        request[1] = {2, 0, 2};
        request[2] = {0, 0, 0};
        request[3] = {1, 0, 0};
        request[4] = {0, 0, 2};
    } else {
        // Guaranteed safe sequence for any N processes
        available = {2, 1, 1};
        for (int i = 0; i < processCount; i++) {
            allocation[i] = {(i % 2) + 1, (i % 3), ((i + 1) % 2)};
            if (i == 0) {
                request[i] = {0, 0, 0}; // First process can execute immediately
            } else {
                request[i] = {1, 0, 0}; // Successive processes finish sequentially
            }
        }
    }
}

void DeadlockManager::loadDeadlockScenario() {
    resourceCount = 3;
    if (processCount <= 0) {
        processIds = {1, 2, 3};
        processNames = {"P1", "P2", "P3"};
        processCount = 3;
    }

    allocation.assign(processCount, vector<int>(resourceCount, 0));
    request.assign(processCount, vector<int>(resourceCount, 0));
    available.assign(resourceCount, 0);

    if (processCount == 5) {
        // Classic Silberschatz 5-process deadlocked state
        available = {0, 0, 0};
        allocation[0] = {0, 1, 0};
        allocation[1] = {2, 0, 0};
        allocation[2] = {3, 0, 3};
        allocation[3] = {2, 1, 1};
        allocation[4] = {0, 0, 2};

        request[0] = {0, 0, 0};
        request[1] = {2, 0, 2};
        request[2] = {0, 0, 1}; // Circular wait
        request[3] = {1, 0, 0};
        request[4] = {0, 0, 2};
    } else {
        // Circular wait for N processes:
        // Available is {0, 0, 0}.
        // Process i holds R(i % 3) and requests R((i + 1) % 3)
        available = {0, 0, 0};
        for (int i = 0; i < processCount; i++) {
            int held = i % resourceCount;
            int req = (i + 1) % resourceCount;
            allocation[i][held] = 1;
            request[i][req] = 1;
        }
    }
}

void DeadlockManager::reset() {
    loadSafeScenario();
}
