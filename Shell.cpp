#include "Shell.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;

// CONSTRUCTOR

Shell::Shell(ProcessManager* pm,
             CPU* processor,
             Scheduler* sched,
             MemoryManager* memory,
             FileManager* files,
             DeadlockManager* deadlock) {

    processManager = pm;
    cpu = processor;
    scheduler = sched;
    memoryManager = memory;

    fileManager = files;
    deadlockManager = deadlock;
}

// HELP

void Shell::help() {

    cout << "\n========================================\n";
    cout << "          MIMIC OS SHELL\n";
    cout << "========================================\n";

    cout << "\nPROCESS COMMANDS\n";
    cout << "  ps                                              Display processes\n";
    cout << "  pcb                                             Display PCBs\n";
    cout << "  create_process <pid> <name> <arr> <burst> <pri> Create process\n";
    cout << "  kill <pid>                                      Terminate process\n";
    cout << "  delete_process <pid>                            Delete process\n";

    cout << "\nCPU COMMANDS\n";
    cout << "  cpu                                             Display CPU status\n";

    cout << "\nSCHEDULING COMMANDS\n";
    cout << "  fcfs                                            Run FCFS\n";
    cout << "  sjf                                             Run SJF\n";
    cout << "  priority                                        Run Priority Scheduling\n";
    cout << "  rr <quantum>                                    Run Round Robin\n";

    cout << "\nMEMORY COMMANDS\n";
    cout << "  memory                                          Display memory\n";
    cout << "  firstfit <pid> <size>                           Allocate First Fit\n";
    cout << "  bestfit <pid> <size>                            Allocate Best Fit\n";
    cout << "  worstfit <pid> <size>                           Allocate Worst Fit\n";
    cout << "  free <pid>                                      Free process memory\n";

    cout << "\nFILE SYSTEM COMMANDS\n";
    cout << "  touch <file>                                    Create file\n";
    cout << "  rm <file>                                       Delete file\n";
    cout << "  write <file> <content>                          Write to file\n";
    cout << "  cat <file>                                      Read file\n";
    cout << "  mkdir <directory>                               Create directory\n";
    cout << "  rmdir <directory>                               Delete directory\n";
    cout << "  ls                                              List files\n";
    cout << "  dirs                                            List directories\n";

    cout << "\nSYSTEM COMMANDS\n";
    cout << "  deadlock                                        Detect deadlock\n";
    cout << "  clear                                           Clear screen\n";
    cout << "  help                                            Show commands\n";
    cout << "  exit                                            Exit Mimic OS\n";

    cout << "\n========================================\n";
}

// COMMAND EXECUTION

void Shell::executeCommand(string command) {

    stringstream ss(command);

    string cmd;

    ss >> cmd;

    if (cmd.empty()) return;

    // PROCESS

    if (cmd == "ps") {

        processManager->displayProcesses();
    }

    else if (cmd == "pcb") {

        processManager->displayPCBs();
    }

    else if (cmd == "create_process") {

        int pid;
        string name;
        int arrival, burst, priority;

        if (ss >> pid >> name >> arrival >> burst >> priority) {
            processManager->createProcess(pid, name, arrival, burst, priority);
        } else {
            cout << "Usage: create_process <pid> <name> <arrival> <burst> <priority>\n";
        }
    }

    else if (cmd == "kill") {

        int pid;

        ss >> pid;

        processManager->terminateProcess(pid);
        cout << "Process " << pid << " terminated.\n";
    }

    else if (cmd == "delete_process") {

        int pid;

        ss >> pid;

        if (processManager->deleteProcess(pid))
            cout << "Process deleted.\n";
        else
            cout << "Process not found.\n";
    }


    //CPU

    else if (cmd == "cpu") {

        cpu->displayStatus();
    }


    //SCHEDULING

    else if (cmd == "fcfs") {

        scheduler->FCFS();
    }

    else if (cmd == "sjf") {

        scheduler->SJF();
    }

    else if (cmd == "priority") {

        scheduler->PriorityScheduling();
    }

    else if (cmd == "rr") {

        int quantum;

        if (ss >> quantum && quantum > 0)
            scheduler->RoundRobin(quantum);
        else
            cout << "Invalid quantum. Usage: rr <quantum>\n";
    }


    //MEMORY

    else if (cmd == "memory") {

        memoryManager->displayMemory();
    }

    else if (cmd == "firstfit") {

        int pid;
        int size;

        if (ss >> pid >> size)
            memoryManager->firstFit(pid, size);
        else
            cout << "Usage: firstfit <pid> <size>\n";
    }

    else if (cmd == "bestfit") {

        int pid;
        int size;

        if (ss >> pid >> size)
            memoryManager->bestFit(pid, size);
        else
            cout << "Usage: bestfit <pid> <size>\n";
    }

    else if (cmd == "worstfit") {

        int pid;
        int size;

        if (ss >> pid >> size)
            memoryManager->worstFit(pid, size);
        else
            cout << "Usage: worstfit <pid> <size>\n";
    }

    else if (cmd == "free") {

        int pid;

        if (ss >> pid)
            memoryManager->freeMemory(pid);
        else
            cout << "Usage: free <pid>\n";
    }


    //FILE SYSTEM

    else if (cmd == "touch") {

        string file;

        if (ss >> file)
            fileManager->createFile(file);
        else
            cout << "Usage: touch <filename>\n";
    }

    else if (cmd == "rm") {

        string file;

        if (ss >> file)
            fileManager->deleteFile(file);
        else
            cout << "Usage: rm <filename>\n";
    }

    else if (cmd == "cat") {

        string file;

        if (ss >> file)
            fileManager->readFile(file);
        else
            cout << "Usage: cat <filename>\n";
    }

    else if (cmd == "write") {

        string file;
        string content;

        if (ss >> file) {
            getline(ss, content);
            if (!content.empty() && content[0] == ' ')
                content.erase(0, 1);

            fileManager->writeFile(file, content);
        } else {
            cout << "Usage: write <filename> <content>\n";
        }
    }

    else if (cmd == "mkdir") {

        string directory;

        if (ss >> directory)
            fileManager->createDirectory(directory);
        else
            cout << "Usage: mkdir <directory>\n";
    }

    else if (cmd == "rmdir") {

        string directory;

        if (ss >> directory)
            fileManager->deleteDirectory(directory);
        else
            cout << "Usage: rmdir <directory>\n";
    }

    else if (cmd == "ls") {

        fileManager->listFiles();
    }

    else if (cmd == "dirs") {

        fileManager->listDirectories();
    }


    //DEADLOCK

    else if (cmd == "deadlock") {

        deadlockManager->display();
        deadlockManager->detectDeadlock();
    }


    //SYSTEM

    else if (cmd == "clear") {

        system("cls");
    }

    else if (cmd == "help") {

        help();
    }

    else if (cmd == "exit") {

        cout << "Session exit requested.\n";
    }

    else {

        cout << "Unknown command: '" << cmd << "'\n";
        cout << "Type 'help' for available commands.\n";
    }
}
// STRING EXECUTION FOR WEB TERMINAL

string Shell::executeCommandString(const string& command) {
    if (command == "clear") {
        return "__CLEAR__";
    }

    stringstream buffer;
    streambuf* oldCout = cout.rdbuf(buffer.rdbuf());

    executeCommand(command);

    cout.rdbuf(oldCout);
    return buffer.str();
}

// START SHELL

void Shell::start() {

    string command;

    cout << "\n";
    cout << "\n";
    cout << "MIMIC OS SHELL\n";
    cout << "\n";

    cout << "Type 'help' to see available commands.\n";
    cout << "Type 'exit' to shutdown Mimic OS.\n";


    while (true) {

        cout << "\n";
        cout << "mimic> ";

        getline(cin, command);

        if (command == "exit") {

            cout << "\nShutting down Mimic OS...\n";

            break;
        }

        if (command.empty())
            continue;

        executeCommand(command);
    }
}
