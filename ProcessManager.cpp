#include "ProcessManager.h"
#include <iostream>
using namespace std;

void ProcessManager::createProcess(int pid, string name,
                                   int arrivalTime, int burstTime,
                                   int priority) {

    if (findProcess(pid) != nullptr) {
        cout << "Process with PID " << pid << " already exists.\n";
        return;
    }

    Process newProcess(pid, name, arrivalTime, burstTime, priority);

    processes.push_back(newProcess);

    PCB newPCB(newProcess);
    pcbs.push_back(newPCB);

    cout << "\nProcess created successfully!\n";
    newProcess.display();
}

void ProcessManager::terminateProcess(int pid) {

    Process* process = findProcess(pid);

    if (process == nullptr) {
        cout << "Process not found.\n";
        return;
    }

    process->setState("Terminated");

    for (PCB& pcb : pcbs) {
        if (pcb.getPID() == pid) {
            pcb.setState("Terminated");
        }
    }
}

void ProcessManager::changeState(int pid, string newState) {

    Process* process = findProcess(pid);

    if (process == nullptr)
        return;

    process->setState(newState);

    for (PCB& pcb : pcbs) {
        if (pcb.getPID() == pid) {
            pcb.setState(newState);
        }
    }
}

Process* ProcessManager::findProcess(int pid) {

    for (Process& process : processes) {
        if (process.getPID() == pid)
            return &process;
    }

    return nullptr;
}

vector<Process>& ProcessManager::getProcesses() {
    return processes;
}

void ProcessManager::displayProcesses() const {

    cout << "\nPROCESS TABLE\n";

    for (const Process& process : processes) {
        cout << "PID: " << process.getPID()
             << " | Name: " << process.getName()
             << " | Arrival: " << process.getArrivalTime()
             << " | Burst: " << process.getBurstTime()
             << " | Priority: " << process.getPriority()
             << " | State: " << process.getState()
             << endl;
    }

    cout << "\n";
}

void ProcessManager::displayPCBs() const {

    cout << "\nPCB TABLE\n";

    for (const PCB& pcb : pcbs) {
        cout << "PID: " << pcb.getPID()
             << " | State: " << pcb.getState()
             << " | Priority: " << pcb.getPriority()
             << " | PC: " << pcb.getProgramCounter()
             << endl;
    }

    cout << "\n";
}

bool ProcessManager::deleteProcess(int pid) {
    bool found = false;
    for (size_t i = 0; i < processes.size(); i++) {
        if (processes[i].getPID() == pid) {
            processes.erase(processes.begin() + i);
            found = true;
            break;
        }
    }
    for (size_t i = 0; i < pcbs.size(); i++) {
        if (pcbs[i].getPID() == pid) {
            pcbs.erase(pcbs.begin() + i);
            break;
        }
    }
    return found;
}

void ProcessManager::clear() {
    processes.clear();
    pcbs.clear();
}

void ProcessManager::resetToDefaults() {
    clear();
    createProcess(1, "P1", 0, 5, 2);
    createProcess(2, "P2", 1, 3, 1);
    createProcess(3, "P3", 2, 2, 3);
}
