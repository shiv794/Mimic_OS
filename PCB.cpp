#include "PCB.h"
#include <iostream>

using namespace std;

PCB::PCB(const Process& process) {

    pid = process.getPID();
    priority = process.getPriority();

    programCounter = 0;

    state = process.getState();
}

int PCB::getPID() const {
    return pid;
}

int PCB::getProgramCounter() const {
    return programCounter;
}

string PCB::getState() const {
    return state;
}

int PCB::getPriority() const {
    return priority;
}

void PCB::setState(string newState) {
    state = newState;
}

void PCB::setProgramCounter(int pc) {
    programCounter = pc;
}

void PCB::display() const {

    cout << "\n========== PCB ==========\n";

    cout << "PID: " << pid << endl;
    cout << "State: " << state << endl;
    cout << "Priority: " << priority << endl;
    cout << "Program Counter: " << programCounter << endl;

    cout << "=========================\n";
}