#ifndef SHELL_H
#define SHELL_H

#include "ProcessManager.h"
#include "CPU.h"
#include "Scheduler.h"
#include "MemoryManager.h"
#include "FileManager.h"
#include "DeadlockManager.h"
#include <string>

using namespace std;

class Shell {

private:

    ProcessManager* processManager;
    CPU* cpu;
    Scheduler* scheduler;
    MemoryManager* memoryManager;

    FileManager* fileManager;
    DeadlockManager* deadlockManager;

public:

    Shell(ProcessManager* pm,
          CPU* cpu,
          Scheduler* scheduler,
          MemoryManager* memory,
          FileManager* files,
          DeadlockManager* deadlock);

    void start();

    // String-based execution for Web Terminal and headless interaction
    string executeCommandString(const string& command);

private:

    void help();
    void executeCommand(string command);
};

#endif
