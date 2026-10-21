#include "Process.h"
#include <iostream>

using namespace std;

Process::Process(int id, string processName, int arrival,
                 int burst, int processPriority) {

    pid = id;
    name = processName;
    arrivalTime = arrival;
    burstTime = burst;
    priority = processPriority;

    state = "New";
}

int Process::getPID() const {
    return pid;
}

string Process::getName() const {
    return name;
}

int Process::getArrivalTime() const {
    return arrivalTime;
}

int Process::getBurstTime() const {
    return burstTime;
}

int Process::getPriority() const {
    return priority;
}

string Process::getState() const {
    return state;
}

void Process::setState(string newState) {
    state = newState;
}

void Process::display() const {

    cout << "PID: " << pid << endl;
    cout << "Name: " << name << endl;
    cout << "Arrival Time: " << arrivalTime << endl;
    cout << "Burst Time: " << burstTime << endl;
    cout << "Priority: " << priority << endl;
    cout << "State: " << state << endl;
}