#ifndef PROCESS_H
#define PROCESS_H

#include <string>
using namespace std;

class Process {
private:
    int pid;
    string name;
    int arrivalTime;
    int burstTime;
    int priority;
    string state;

public:
    
    Process(int id, string processName, int arrival,
            int burst, int processPriority);

    int getPID() const;
    string getName() const;
    int getArrivalTime() const;
    int getBurstTime() const;
    int getPriority() const;
    string getState() const;

  
    void setState(string newState);

    void display() const;
};

#endif