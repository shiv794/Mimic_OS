#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <string>
#include "ProcessManager.h"
#include "CPU.h"
#include "MemoryManager.h"
#include "Scheduler.h"
#include "FileManager.h"
#include "DeadlockManager.h"
#include "Shell.h"
#include "SimulationEngine.h"

using namespace std;

class ApiHandler {
private:
    ProcessManager processManager;
    CPU cpu;
    MemoryManager memoryManager;
    Scheduler scheduler;
    FileManager fileManager;
    DeadlockManager deadlockManager;
    Shell shell;

public:
    ApiHandler();

    void handleRequest(const string& method, const string& path,
                       const string& requestBody, string& responseBody,
                       int& statusCode, string& contentType);

private:
    // Core telemetry
    string handleGetStatus();

    // Process endpoints
    string handleGetProcesses();
    string handleCreateProcess(const string& body);
    string handleDeleteProcess(const string& body);
    string handleChangeState(const string& body);
    string handleResetProcesses();

    // CPU endpoints
    string handleGetCPU();
    string handleExecuteCPU(const string& body);
    string handleStopCPU();

    // Memory endpoints
    string handleGetMemory();
    string handleAllocateMemory(const string& body);
    string handleFreeMemory(const string& body);
    string handleResetMemory(const string& body);

    // Scheduling endpoints
    string handleRunSchedule(const string& body);

    // File Management endpoints
    string handleGetFiles();
    string handleCreateFile(const string& body);
    string handleDeleteFile(const string& body);
    string handleWriteFile(const string& body);
    string handleReadFile(const string& body);
    string handleCreateDirectory(const string& body);
    string handleDeleteDirectory(const string& body);
    string handleResetFiles();

    // Deadlock Detection endpoints
    string handleGetDeadlock();
    string handleDetectDeadlock();
    string handleDeadlockScenario(const string& body);
    string handleDeadlockUpdate(const string& body);

    // Shell Terminal endpoint
    string handleShellExecute(const string& body);
};

#endif
