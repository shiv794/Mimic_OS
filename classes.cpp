#include <iostream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

// 1. PROCESS CLASS


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

// 2. PCB CLASS


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


// 3. PROCESS MANAGER CLASS

class ProcessManager {
private:
    vector<Process> processes;
    vector<PCB> pcbs;

public:
    void createProcess(int pid, string name,
                       int arrivalTime, int burstTime,
                       int priority);

    void terminateProcess(int pid);

    void changeState(int pid, string newState);

    void displayProcesses() const;
    void displayPCBs() const;

    Process* findProcess(int pid);

    vector<Process>& getProcesses();
};

// 4. SCHEDULER CLASS


class Scheduler {
private:
    ProcessManager* processManager;

public:
    Scheduler(ProcessManager* manager);

    void FCFS();
    void SJF();
    void PriorityScheduling();
    void RoundRobin(int timeQuantum);

    void displayGanttChart();
    void displayStatistics();
};

// 5. MEMORY MANAGER CLASS


class MemoryManager {
private:
    int totalMemory;
    vector<bool> memory;

public:
    MemoryManager(int size);

    void allocateMemory(int pid, int size);
    void deallocateMemory(int pid);

    void firstFit(int pid, int size);
    void bestFit(int pid, int size);
    void worstFit(int pid, int size);

    void displayMemory();
};

// 6. FILE CLASS


class File {
private:
    string fileName;
    string content;
    int size;

public:
    File(string name);

    void write(string data);
    void read() const;

    string getFileName() const;
    int getSize() const;
};


class Directory {
private:
    string directoryName;
    vector<File> files;

public:

    Directory(string name);

    void createFile(string name);
    void deleteFile(string name);
    void displayFiles() const;
};


// 8. FILE SYSTEM CLASS

class FileSystem {
private:
    vector<Directory> directories;

public:

    void createDirectory(string name);
    void deleteDirectory(string name);

    void createFile(string directoryName, string fileName);
    void deleteFile(string directoryName, string fileName);

    void displayFileSystem() const;
};

// 9. DEADLOCK MANAGER CLASS

class DeadlockManager {
private:
    vector<vector<int>> allocation;
    vector<vector<int>> request;
    vector<int> available;

public:
    void allocateResource(int processID, int resourceID);
    void releaseResource(int processID, int resourceID);

    bool detectDeadlock();

    void displayResourceStatus() const;
};

// 10. CPU CLASS


class CPU {
private:
    Process* currentProcess;
    bool busy;

public:
    CPU();

    void executeProcess(Process* process);
    void stopProcess();

    bool isBusy() const;
    void displayStatus() const;
};

// 11. SHELL CLASS

class Shell {
private:
    ProcessManager* processManager;
    Scheduler* scheduler;
    MemoryManager* memoryManager;
    FileSystem* fileSystem;
    DeadlockManager* deadlockManager;
    CPU* cpu;

public:

    Shell(ProcessManager* pm,
          Scheduler* scheduler,
          MemoryManager* mm,
          FileSystem* fs,
          DeadlockManager* dm,
          CPU* cpu);

    void start();
    void executeCommand(string command);
    void displayHelp();
};