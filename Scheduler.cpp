
#include "Scheduler.h"
#include <iostream>
#include <algorithm>
#include <queue>

using namespace std;

Scheduler::Scheduler(ProcessManager* manager, CPU* processor)
{
    processManager = manager;
    cpu = processor;
}

// ================= FCFS =================

void Scheduler::FCFS()
{
    vector<Process>& processes = processManager->getProcesses();

    sort(processes.begin(), processes.end(),
         [](Process& a, Process& b)
         {
             return a.getArrivalTime() < b.getArrivalTime();
         });

    cout << "\n========== FCFS SCHEDULING ==========\n";

    int currentTime = 0;

    for (Process& process : processes)
    {
        if (currentTime < process.getArrivalTime())
            currentTime = process.getArrivalTime();

        cout << "\nTime " << currentTime
             << " -> CPU selects P"
             << process.getPID() << endl;

        process.setState("Running");
        cpu->executeProcess(&process);

        cout << "Executing P" << process.getPID()
             << " for " << process.getBurstTime()
             << " units.\n";

        currentTime += process.getBurstTime();

        cpu->stopProcess();
        process.setState("Terminated");

        cout << "Time " << currentTime
             << " -> P" << process.getPID()
             << " completed.\n";
    }

    cout << "\n========== GANTT CHART ==========\n";

    for (Process& process : processes)
        cout << "| P" << process.getPID() << " ";

    cout << "|\n=================================\n";
}

// ================= SJF =================

void Scheduler::SJF()
{
    vector<Process>& processes = processManager->getProcesses();

    vector<int> remainingTime;
    int n = processes.size();

    for (Process& process : processes)
    {
        remainingTime.push_back(process.getBurstTime());
        process.setState("Ready");
    }

    int completed = 0;
    int currentTime = 0;

    cout << "\n========== SJF SCHEDULING ==========\n";

    while (completed < n)
    {
        int selected = -1;

        for (int i = 0; i < n; i++)
        {
            if (processes[i].getState() == "Ready" &&
                processes[i].getArrivalTime() <= currentTime)
            {
                if (selected == -1 ||
                    remainingTime[i] < remainingTime[selected])
                {
                    selected = i;
                }
            }
        }

        if (selected == -1)
        {
            currentTime++;
            continue;
        }

        Process* current = &processes[selected];

        cout << "\nTime " << currentTime
             << " -> SJF selects P"
             << current->getPID() << endl;

        current->setState("Running");
        cpu->executeProcess(current);

        cout << "Executing for "
             << remainingTime[selected] << " units.\n";

        currentTime += remainingTime[selected];
        remainingTime[selected] = 0;

        cpu->stopProcess();
        current->setState("Terminated");
        completed++;

        cout << "P" << current->getPID()
             << " completed at time " << currentTime << ".\n";
    }

    cout << "\n========== SJF COMPLETED ==========\n";
}

// ================ PRIORITY SCHEDULING ================

void Scheduler::PriorityScheduling()
{
    vector<Process>& processes = processManager->getProcesses();

    int n = processes.size();
    vector<int> remainingTime;

    for (Process& process : processes)
    {
        remainingTime.push_back(process.getBurstTime());
        process.setState("Ready");
    }

    int completed = 0;
    int currentTime = 0;

    cout << "\n======= PRIORITY SCHEDULING =======\n";
    cout << "Lower priority number = higher priority.\n";

    while (completed < n)
    {
        int selected = -1;

        for (int i = 0; i < n; i++)
        {
            if (processes[i].getState() == "Ready" &&
                processes[i].getArrivalTime() <= currentTime)
            {
                if (selected == -1 ||
                    processes[i].getPriority() <
                        processes[selected].getPriority())
                {
                    selected = i;
                }
            }
        }

        if (selected == -1)
        {
            currentTime++;
            continue;
        }

        Process* current = &processes[selected];

        cout << "\nTime " << currentTime
             << " -> Priority selects P" << current->getPID()
             << " (Priority " << current->getPriority() << ")\n";

        current->setState("Running");
        cpu->executeProcess(current);

        cout << "Executing for "
             << remainingTime[selected] << " units.\n";

        currentTime += remainingTime[selected];
        remainingTime[selected] = 0;

        cpu->stopProcess();
        current->setState("Terminated");
        completed++;

        cout << "P" << current->getPID()
             << " completed at time " << currentTime << ".\n";
    }

    cout << "\n======= PRIORITY SCHEDULING COMPLETED =======\n";
}

// ================ ROUND ROBIN ================

void Scheduler::RoundRobin(int timeQuantum)
{
    if (timeQuantum <= 0)
    {
        cout << "Time quantum must be positive.\n";
        return;
    }

    vector<Process>& processes = processManager->getProcesses();

    int n = processes.size();

    if (n == 0)
    {
        cout << "No processes available.\n";
        return;
    }

    vector<int> remainingTime;

    for (Process& process : processes)
    {
        remainingTime.push_back(process.getBurstTime());
        process.setState("New");
    }

    queue<int> readyQueue;

    int currentTime = 0;
    int completed = 0;

    cout << "\n======= ROUND ROBIN SCHEDULING =======\n";
    cout << "Time Quantum: " << timeQuantum << endl;

    while (completed < n)
    {
        // Enqueue processes that have arrived.
        for (int i = 0; i < n; i++)
        {
            if (processes[i].getState() == "New" &&
                processes[i].getArrivalTime() <= currentTime)
            {
                processes[i].setState("Queued");
                readyQueue.push(i);
            }
        }

        if (readyQueue.empty())
        {
            currentTime++;
            continue;
        }

        int index = readyQueue.front();
        readyQueue.pop();

        Process* current = &processes[index];

        current->setState("Running");

        cout << "\nTime " << currentTime
             << " -> CPU selects P"
             << current->getPID() << endl;

        cpu->executeProcess(current);

        int executionTime = min(timeQuantum, remainingTime[index]);

        cout << "Executing P" << current->getPID()
             << " for " << executionTime << " units.\n";

        currentTime += executionTime;
        remainingTime[index] -= executionTime;

        // Enqueue processes arriving during this time slice.
        for (int i = 0; i < n; i++)
        {
            if (processes[i].getState() == "New" &&
                processes[i].getArrivalTime() <= currentTime)
            {
                processes[i].setState("Queued");
                readyQueue.push(i);
            }
        }

        cpu->stopProcess();

        if (remainingTime[index] == 0)
        {
            current->setState("Terminated");
            completed++;

            cout << "P" << current->getPID()
                 << " completed at time " << currentTime << ".\n";
        }
        else
        {
            current->setState("Queued");

            cout << "P" << current->getPID()
                 << " has " << remainingTime[index]
                 << " units remaining.\n";

            readyQueue.push(index);
        }
    }

    cout << "\n======= ROUND ROBIN COMPLETED =======\n";
}
