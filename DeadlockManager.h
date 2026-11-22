#ifndef DEADLOCK_MANAGER_H
#define DEADLOCK_MANAGER_H

#include <vector>
#include <string>
#include "Process.h"

using namespace std;

struct DeadlockResult {
    bool isDeadlocked;
    vector<int> deadlockedProcesses; // Row indices
    vector<int> deadlockedPids;      // Process IDs
    vector<int> safeSequence;        // Row indices
    vector<int> safeSequencePids;    // Process IDs
    vector<string> safeSequenceNames;// Process Names
};

class DeadlockManager {

private:
    int processCount;
    int resourceCount;

    vector<int> processIds;
    vector<string> processNames;

    vector<vector<int>> allocation;
    vector<vector<int>> request;
    vector<int> available;

public:
    DeadlockManager();

    void initialize(int processes, int resources);
    void syncWithProcesses(const vector<Process>& processes);

    void addProcess(int pid, const string& name);
    bool removeProcess(int pid);
    int findProcessIndex(int pid) const;

    int getPidAt(int index) const;
    string getNameAt(int index) const;

    void setAllocation(int process, int resource, int value);
    void setRequest(int process, int resource, int value);
    void setAvailable(int resource, int value);

    void display();
    bool detectDeadlock();

    // Getters and Helpers for GUI / API
    int getProcessCount() const { return processCount; }
    int getResourceCount() const { return resourceCount; }
    const vector<int>& getProcessIds() const { return processIds; }
    const vector<string>& getProcessNames() const { return processNames; }
    const vector<vector<int>>& getAllocation() const { return allocation; }
    const vector<vector<int>>& getRequest() const { return request; }
    const vector<int>& getAvailable() const { return available; }

    DeadlockResult evaluateDeadlock() const;

    void loadSafeScenario();
    void loadDeadlockScenario();
    void reset();
};

#endif
