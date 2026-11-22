#include "CPU.h"
#include <iostream>

using namespace std;

CPU::CPU() {
    currentProcess = nullptr;
    busy = false;

    lastProcessID = -1;
    lastProcessName = "None";
    lastActivity = "No process executed yet";
}

void CPU::executeProcess(Process* process) {
    if (process == nullptr) {
        return;
    }

    currentProcess = process;
    busy = true;

    lastProcessID = process->getPID();
    lastProcessName = process->getName();
    lastActivity = "Executed process " + lastProcessName;

    cout << "\n[CPU] Executing Process: "
         << lastProcessName
         << " (PID: " << lastProcessID << ")\n";
}

void CPU::stopProcess() {
    currentProcess = nullptr;
    busy = false;
}

bool CPU::isBusy() const {
    return busy;
}

void CPU::displayStatus() const {
    cout << "\n========== CPU STATUS ==========\n";

    if (busy && currentProcess != nullptr) {
        cout << "Current State  : BUSY\n";
        cout << "Current Process: "
             << currentProcess->getName()
             << " (PID: " << currentProcess->getPID() << ")\n";
    } else {
        cout << "Current State  : IDLE\n";
        cout << "Current Process: None\n";
    }

    cout << "Last Executed  : " << lastProcessName << "\n";

    if (lastProcessID != -1) {
        cout << "Last Process PID: " << lastProcessID << "\n";
    }

    cout << "Last Activity  : " << lastActivity << "\n";
    cout << "================================\n";
}

void CPU::reset() {
    currentProcess = nullptr;
    busy = false;
    lastProcessID = -1;
    lastProcessName = "None";
    lastActivity = "No process executed yet";
}
