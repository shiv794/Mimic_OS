#ifndef PCB_H
#define PCB_H

#include "Process.h"

class PCB {
private:
    int pid;
    int programCounter;
    string state;
    int priority;

public:
    PCB(const Process& process);
  
    int getPID() const;
    int getProgramCounter() const;
    string getState() const;
    int getPriority() const;
   
    void setState(string newState);

    void setProgramCounter(int pc);
    
    void display() const;
};

#endif