#ifndef API_HANDLER_H
#define API_HANDLER_H

#include <string>
#include "ProcessManager.h"
#include "CPU.h"
#include "MemoryManager.h"
#include "Scheduler.h"
#include "SimulationEngine.h"

using namespace std;

class ApiHandler {
private:
    ProcessManager processManager;
    CPU cpu;
    MemoryManager memoryManager;
    Scheduler scheduler;

public:
    ApiHandler();

    void handleRequest(const string& method, const string& path,
                       const string& requestBody, string& responseBody,
                       int& statusCode, string& contentType);

private:
    string handleGetStatus();
    string handleGetProcesses();
    string handleCreateProcess(const string& body);
    string handleDeleteProcess(const string& body);
    string handleChangeState(const string& body);
    string handleResetProcesses();

    string handleGetCPU();
    string handleExecuteCPU(const string& body);
    string handleStopCPU();

    string handleGetMemory();
    string handleAllocateMemory(const string& body);
    string handleFreeMemory(const string& body);
    string handleResetMemory();

    string handleRunSchedule(const string& body);
};

#endif

