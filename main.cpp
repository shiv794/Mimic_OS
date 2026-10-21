#include <iostream>
#include <windows.h>
#include <shellapi.h>
#include "ProcessManager.h"
#include "CPU.h"
#include "Scheduler.h"
#include "MemoryManager.h"
#include "HttpServer.h"

using namespace std;

int main(int argc, char* argv[])
{
    // Check if launched with --gui or --server command-line flag
    if (argc > 1 && (string(argv[1]) == "--gui" || string(argv[1]) == "--server"))
    {
        cout << "\n======================================================\n";
        cout << "        MIMIC OS - INTERACTIVE WEB GUI SERVER         \n";
        cout << "======================================================\n";
        cout << "[Mimic OS] Opening browser at http://localhost:8080...\n";
        ShellExecuteA(NULL, "open", "http://localhost:8080", NULL, NULL, SW_SHOWNORMAL);
        HttpServer server(8080, "./web");
        server.start();
        return 0;
    }

    ProcessManager manager;
    CPU cpu;
    Scheduler scheduler(&manager, &cpu);
    MemoryManager memory(1024);

    // Default processes
    manager.createProcess(1, "P1", 0, 5, 2);
    manager.createProcess(2, "P2", 1, 3, 1);
    manager.createProcess(3, "P3", 2, 2, 3);

    int choice;

    do
    {
        cout << "\n========== MIMIC OS ==========\n";
        cout << "1. Create Process\n";
        cout << "2. Display Processes\n";
        cout << "3. Display PCBs\n";
        cout << "4. CPU Status\n";
        cout << "5. CPU Scheduling\n";
        cout << "6. Memory Management\n";
        cout << "7. Launch Interactive Web GUI (http://localhost:8080)\n";
        cout << "8. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            int pid;
            string name;
            int arrivalTime;
            int burstTime;
            int priority;

            cout << "\n--- Create New Process ---\n";

            cout << "Enter Process ID: ";
            cin >> pid;

            cout << "Enter Process Name: ";
            cin >> name;

            cout << "Enter Arrival Time: ";
            cin >> arrivalTime;

            cout << "Enter Burst Time: ";
            cin >> burstTime;

            cout << "Enter Priority: ";
            cin >> priority;

            if (pid <= 0 || arrivalTime < 0 || burstTime <= 0 || priority <= 0)
            {
                cout << "Invalid process details.\n";
                break;
            }

            manager.createProcess(pid, name, arrivalTime, burstTime, priority);

            cout << "Process created successfully.\n";
            break;
        }

        case 2:
            manager.displayProcesses();
            break;

        case 3:
            manager.displayPCBs();
            break;

        case 4:
            cpu.displayStatus();
            break;

        case 5:
        {
            int option;

            cout << "\n--- CPU Scheduling ---\n";
            cout << "1. FCFS\n";
            cout << "2. SJF\n";
            cout << "3. Priority Scheduling\n";
            cout << "4. Round Robin\n";
            cout << "Enter choice: ";
            cin >> option;

            switch (option)
            {
            case 1:
                scheduler.FCFS();
                break;

            case 2:
                scheduler.SJF();
                break;

            case 3:
                scheduler.PriorityScheduling();
                break;

            case 4:
            {
                int quantum;

                cout << "Enter time quantum: ";
                cin >> quantum;

                if (quantum > 0)
                    scheduler.RoundRobin(quantum);
                else
                    cout << "Quantum must be positive.\n";

                break;
            }

            default:
                cout << "Invalid choice.\n";
            }

            break;
        }

        case 6:
        {
            int option;

            cout << "\n--- Memory Management ---\n";
            cout << "1. First Fit\n";
            cout << "2. Best Fit\n";
            cout << "3. Worst Fit\n";
            cout << "4. Display Memory\n";
            cout << "5. Free Memory\n";
            cout << "Enter choice: ";
            cin >> option;

            switch (option)
            {
            case 1:
            case 2:
            case 3:
            {
                int pid, size;

                cout << "Enter process ID: ";
                cin >> pid;

                cout << "Enter memory required (MB): ";
                cin >> size;

                if (pid <= 0 || size <= 0)
                {
                    cout << "PID and size must be positive.\n";
                    break;
                }

                if (option == 1)
                    memory.firstFit(pid, size);
                else if (option == 2)
                    memory.bestFit(pid, size);
                else
                    memory.worstFit(pid, size);

                break;
            }

            case 4:
                memory.displayMemory();
                break;

            case 5:
            {
                int pid;

                cout << "Enter process ID to free: ";
                cin >> pid;

                memory.freeMemory(pid);
                break;
            }

            default:
                cout << "Invalid choice.\n";
            }

            break;
        }

        case 7:
        {
            cout << "\n[Mimic OS] Starting Interactive Web GUI at http://localhost:8080...\n";
            ShellExecuteA(NULL, "open", "http://localhost:8080", NULL, NULL, SW_SHOWNORMAL);
            HttpServer server(8080, "./web");
            server.start();
            break;
        }

        case 8:
            cout << "Exiting Mimic OS...\n";
            break;

        default:
            cout << "Invalid choice. Try again.\n";
        }

    } while (choice != 8);

    return 0;
}

