#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "ProcessManager.h"
#include "CPU.h"
#include <vector>
using namespace std;

class Scheduler {
private:
    ProcessManager* processManager;
    CPU* cpu;

public:
    Scheduler(ProcessManager* manager, CPU* processor);

    void FCFS();
    void SJF();
    void PriorityScheduling();
    void RoundRobin(int timeQuantum);
};

#endif