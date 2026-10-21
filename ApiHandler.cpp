#include "ApiHandler.h"
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

// Quick JSON value extractors
static string extractStringField(const string& json, const string& key, const string& defaultVal = "") {
    string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == string::npos) return defaultVal;

    size_t colon = json.find(':', pos + searchKey.length());
    if (colon == string::npos) return defaultVal;

    size_t start = json.find('"', colon + 1);
    if (start == string::npos) return defaultVal;

    size_t end = json.find('"', start + 1);
    if (end == string::npos) return defaultVal;

    return json.substr(start + 1, end - start - 1);
}

static int extractIntField(const string& json, const string& key, int defaultVal = 0) {
    string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == string::npos) return defaultVal;

    size_t colon = json.find(':', pos + searchKey.length());
    if (colon == string::npos) return defaultVal;

    size_t start = json.find_first_of("-0123456789", colon + 1);
    if (start == string::npos) return defaultVal;

    size_t end = json.find_first_not_of("-0123456789", start + 1);
    string numStr = (end == string::npos) ? json.substr(start) : json.substr(start, end - start);

    try {
        return stoi(numStr);
    } catch (...) {
        return defaultVal;
    }
}

ApiHandler::ApiHandler()
    : memoryManager(1024),
      scheduler(&processManager, &cpu)
{
    // Initialize standard default processes
    processManager.createProcess(1, "P1", 0, 5, 2);
    processManager.createProcess(2, "P2", 1, 3, 1);
    processManager.createProcess(3, "P3", 2, 2, 3);
}

void ApiHandler::handleRequest(const string& method, const string& path,
                               const string& requestBody, string& responseBody,
                               int& statusCode, string& contentType) {
    contentType = "application/json; charset=utf-8";
    statusCode = 200;

    if (method == "GET") {
        if (path == "/api/status") {
            responseBody = handleGetStatus();
        } else if (path == "/api/processes") {
            responseBody = handleGetProcesses();
        } else if (path == "/api/cpu") {
            responseBody = handleGetCPU();
        } else if (path == "/api/memory") {
            responseBody = handleGetMemory();
        } else {
            statusCode = 404;
            responseBody = "{\"error\": \"Endpoint not found\"}";
        }
    } else if (method == "POST") {
        if (path == "/api/processes/create") {
            responseBody = handleCreateProcess(requestBody);
        } else if (path == "/api/processes/delete") {
            responseBody = handleDeleteProcess(requestBody);
        } else if (path == "/api/processes/state") {
            responseBody = handleChangeState(requestBody);
        } else if (path == "/api/processes/reset") {
            responseBody = handleResetProcesses();
        } else if (path == "/api/cpu/execute") {
            responseBody = handleExecuteCPU(requestBody);
        } else if (path == "/api/cpu/stop") {
            responseBody = handleStopCPU();
        } else if (path == "/api/memory/allocate") {
            responseBody = handleAllocateMemory(requestBody);
        } else if (path == "/api/memory/free") {
            responseBody = handleFreeMemory(requestBody);
        } else if (path == "/api/memory/reset") {
            responseBody = handleResetMemory();
        } else if (path == "/api/schedule/run") {
            responseBody = handleRunSchedule(requestBody);
        } else {
            statusCode = 404;
            responseBody = "{\"error\": \"Endpoint not found\"}";
        }
    } else {
        statusCode = 405;
        responseBody = "{\"error\": \"Method not allowed\"}";
    }
}

string ApiHandler::handleGetStatus() {
    int total = processManager.getProcesses().size();
    int countNew = 0, countReady = 0, countRunning = 0, countWaiting = 0, countTerminated = 0;

    for (const Process& p : processManager.getProcesses()) {
        string s = p.getState();
        if (s == "New") countNew++;
        else if (s == "Ready") countReady++;
        else if (s == "Running") countRunning++;
        else if (s == "Waiting") countWaiting++;
        else if (s == "Terminated") countTerminated++;
        else countReady++;
    }

    int totalMem = memoryManager.getTotalMemory();
    int usedMem = memoryManager.getUsedMemory();
    int freeMem = memoryManager.getFreeMemory();
    double usedPercent = totalMem > 0 ? ((double)usedMem * 100.0 / totalMem) : 0;

    ostringstream oss;
    oss << "{\n";
    oss << "  \"system\": \"Mimic OS\",\n";
    oss << "  \"status\": \"online\",\n";
    oss << "  \"version\": \"1.0.0\",\n";
    oss << "  \"cpu\": {\n";
    oss << "    \"busy\": " << (cpu.isBusy() ? "true" : "false") << ",\n";
    if (cpu.getCurrentProcess() != nullptr) {
        oss << "    \"currentPid\": " << cpu.getCurrentProcess()->getPID() << ",\n";
        oss << "    \"currentName\": \"" << cpu.getCurrentProcess()->getName() << "\",\n";
        oss << "    \"remainingBurst\": " << cpu.getCurrentProcess()->getBurstTime() << ",\n";
        oss << "    \"state\": \"" << cpu.getCurrentProcess()->getState() << "\"\n";
    } else {
        oss << "    \"currentPid\": -1,\n";
        oss << "    \"currentName\": \"None\",\n";
        oss << "    \"remainingBurst\": 0,\n";
        oss << "    \"state\": \"IDLE\"\n";
    }
    oss << "  },\n";
    oss << "  \"processes\": {\n";
    oss << "    \"total\": " << total << ",\n";
    oss << "    \"new\": " << countNew << ",\n";
    oss << "    \"ready\": " << countReady << ",\n";
    oss << "    \"running\": " << countRunning << ",\n";
    oss << "    \"waiting\": " << countWaiting << ",\n";
    oss << "    \"terminated\": " << countTerminated << "\n";
    oss << "  },\n";
    oss << "  \"memory\": {\n";
    oss << "    \"total\": " << totalMem << ",\n";
    oss << "    \"used\": " << usedMem << ",\n";
    oss << "    \"free\": " << freeMem << ",\n";
    oss << "    \"usedPercent\": " << usedPercent << "\n";
    oss << "  }\n";
    oss << "}";

    return oss.str();
}

string ApiHandler::handleGetProcesses() {
    const vector<Process>& procList = processManager.getProcesses();
    const vector<PCB>& pcbList = processManager.getPCBs();

    ostringstream oss;
    oss << "{\n";
    oss << "  \"processes\": [\n";
    for (size_t i = 0; i < procList.size(); i++) {
        const Process& p = procList[i];
        oss << "    {\"pid\": " << p.getPID()
            << ", \"name\": \"" << p.getName() << "\""
            << ", \"arrivalTime\": " << p.getArrivalTime()
            << ", \"burstTime\": " << p.getBurstTime()
            << ", \"priority\": " << p.getPriority()
            << ", \"state\": \"" << p.getState() << "\"}";
        if (i + 1 < procList.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    oss << "  \"pcbs\": [\n";
    for (size_t i = 0; i < pcbList.size(); i++) {
        const PCB& pcb = pcbList[i];
        oss << "    {\"pid\": " << pcb.getPID()
            << ", \"state\": \"" << pcb.getState() << "\""
            << ", \"priority\": " << pcb.getPriority()
            << ", \"programCounter\": " << pcb.getProgramCounter() << "}";
        if (i + 1 < pcbList.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ]\n";
    oss << "}";

    return oss.str();
}

string ApiHandler::handleCreateProcess(const string& body) {
    int pid = extractIntField(body, "pid", 0);
    string name = extractStringField(body, "name", "");
    int arrivalTime = extractIntField(body, "arrivalTime", 0);
    int burstTime = extractIntField(body, "burstTime", 1);
    int priority = extractIntField(body, "priority", 1);

    if (pid <= 0) {
        // Auto assign pid
        int maxPid = 0;
        for (const Process& p : processManager.getProcesses()) {
            if (p.getPID() > maxPid) maxPid = p.getPID();
        }
        pid = maxPid + 1;
    }

    if (name.empty()) {
        name = "P" + to_string(pid);
    }

    if (burstTime <= 0) burstTime = 1;
    if (arrivalTime < 0) arrivalTime = 0;
    if (priority <= 0) priority = 1;

    processManager.createProcess(pid, name, arrivalTime, burstTime, priority);

    return "{\"success\": true, \"message\": \"Process created\", \"pid\": " + to_string(pid) + "}";
}

string ApiHandler::handleDeleteProcess(const string& body) {
    int pid = extractIntField(body, "pid", 0);
    bool deleted = processManager.deleteProcess(pid);
    return "{\"success\": " + string(deleted ? "true" : "false") + "}";
}

string ApiHandler::handleChangeState(const string& body) {
    int pid = extractIntField(body, "pid", 0);
    string state = extractStringField(body, "state", "Ready");
    processManager.changeState(pid, state);
    return "{\"success\": true}";
}

string ApiHandler::handleResetProcesses() {
    processManager.resetToDefaults();
    cpu.reset();
    return "{\"success\": true, \"message\": \"Reset to default processes P1, P2, P3\"}";
}

string ApiHandler::handleGetCPU() {
    ostringstream oss;
    oss << "{\n";
    oss << "  \"busy\": " << (cpu.isBusy() ? "true" : "false") << ",\n";
    if (cpu.getCurrentProcess() != nullptr) {
        oss << "  \"currentProcess\": {\n";
        oss << "    \"pid\": " << cpu.getCurrentProcess()->getPID() << ",\n";
        oss << "    \"name\": \"" << cpu.getCurrentProcess()->getName() << "\",\n";
        oss << "    \"burstTime\": " << cpu.getCurrentProcess()->getBurstTime() << ",\n";
        oss << "    \"priority\": " << cpu.getCurrentProcess()->getPriority() << ",\n";
        oss << "    \"state\": \"" << cpu.getCurrentProcess()->getState() << "\"\n";
        oss << "  },\n";
    } else {
        oss << "  \"currentProcess\": null,\n";
    }
    oss << "  \"lastProcessID\": " << cpu.getLastProcessID() << ",\n";
    oss << "  \"lastProcessName\": \"" << cpu.getLastProcessName() << "\",\n";
    oss << "  \"lastActivity\": \"" << cpu.getLastActivity() << "\"\n";
    oss << "}";
    return oss.str();
}

string ApiHandler::handleExecuteCPU(const string& body) {
    int pid = extractIntField(body, "pid", 0);
    Process* p = processManager.findProcess(pid);
    if (!p) {
        return "{\"success\": false, \"message\": \"Process not found\"}";
    }
    processManager.changeState(pid, "Running");
    cpu.executeProcess(p);
    return "{\"success\": true, \"message\": \"Executing process " + p->getName() + "\"}";
}

string ApiHandler::handleStopCPU() {
    if (cpu.isBusy() && cpu.getCurrentProcess() != nullptr) {
        processManager.changeState(cpu.getCurrentProcess()->getPID(), "Ready");
    }
    cpu.stopProcess();
    return "{\"success\": true, \"message\": \"CPU stopped\"}";
}

string ApiHandler::handleGetMemory() {
    const vector<MemoryBlock>& blocks = memoryManager.getMemoryBlocks();
    ostringstream oss;
    oss << "{\n";
    oss << "  \"totalMemory\": " << memoryManager.getTotalMemory() << ",\n";
    oss << "  \"usedMemory\": " << memoryManager.getUsedMemory() << ",\n";
    oss << "  \"freeMemory\": " << memoryManager.getFreeMemory() << ",\n";
    oss << "  \"blocks\": [\n";
    for (size_t i = 0; i < blocks.size(); i++) {
        const MemoryBlock& b = blocks[i];
        oss << "    {\"startAddress\": " << b.startAddress
            << ", \"size\": " << b.size
            << ", \"pid\": " << b.pid
            << ", \"allocated\": " << (b.allocated ? "true" : "false") << "}";
        if (i + 1 < blocks.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ]\n";
    oss << "}";
    return oss.str();
}

string ApiHandler::handleAllocateMemory(const string& body) {
    string algorithm = extractStringField(body, "algorithm", "firstFit");
    int pid = extractIntField(body, "pid", 1);
    int size = extractIntField(body, "size", 100);

    if (pid <= 0 || size <= 0) {
        return "{\"success\": false, \"message\": \"PID and size must be positive\"}";
    }

    // Inspect blocks before allocation
    vector<MemoryBlock> beforeBlocks = memoryManager.getMemoryBlocks();

    // Determine why a block will be selected (for educational explanation)
    int targetIndex = -1;
    string reason = "";

    if (algorithm == "firstFit" || algorithm == "First Fit") {
        for (size_t i = 0; i < beforeBlocks.size(); i++) {
            if (!beforeBlocks[i].allocated && beforeBlocks[i].size >= size) {
                targetIndex = (int)i;
                reason = "First Fit scanned linearly and selected Block #" + to_string(i) +
                         " at address " + to_string(beforeBlocks[i].startAddress) +
                         " (size: " + to_string(beforeBlocks[i].size) + " MB) because it was the FIRST free hole >= " + to_string(size) + " MB.";
                break;
            }
        }
        memoryManager.firstFit(pid, size);
    } else if (algorithm == "bestFit" || algorithm == "Best Fit") {
        int smallestDiff = 9999999;
        for (size_t i = 0; i < beforeBlocks.size(); i++) {
            if (!beforeBlocks[i].allocated && beforeBlocks[i].size >= size) {
                int diff = beforeBlocks[i].size - size;
                if (diff < smallestDiff) {
                    smallestDiff = diff;
                    targetIndex = (int)i;
                }
            }
        }
        if (targetIndex != -1) {
            reason = "Best Fit searched all free blocks and selected Block #" + to_string(targetIndex) +
                     " at address " + to_string(beforeBlocks[targetIndex].startAddress) +
                     " (size: " + to_string(beforeBlocks[targetIndex].size) + " MB) because it leaves the SMALLEST remaining fragment (" + to_string(smallestDiff) + " MB slack).";
        }
        memoryManager.bestFit(pid, size);
    } else if (algorithm == "worstFit" || algorithm == "Worst Fit") {
        int largestSize = -1;
        for (size_t i = 0; i < beforeBlocks.size(); i++) {
            if (!beforeBlocks[i].allocated && beforeBlocks[i].size >= size) {
                if (beforeBlocks[i].size > largestSize) {
                    largestSize = beforeBlocks[i].size;
                    targetIndex = (int)i;
                }
            }
        }
        if (targetIndex != -1) {
            reason = "Worst Fit searched all free blocks and selected Block #" + to_string(targetIndex) +
                     " at address " + to_string(beforeBlocks[targetIndex].startAddress) +
                     " (size: " + to_string(beforeBlocks[targetIndex].size) + " MB) because it was the LARGEST available free hole.";
        }
        memoryManager.worstFit(pid, size);
    }

    // Verify allocation succeeded by inspecting current blocks
    bool success = false;
    int startAddr = -1;
    for (const MemoryBlock& b : memoryManager.getMemoryBlocks()) {
        if (b.allocated && b.pid == pid) {
            success = true;
            startAddr = b.startAddress;
            break;
        }
    }

    if (!success) {
        reason = "Allocation failed: Not enough contiguous memory available for P" + to_string(pid) + " (" + to_string(size) + " MB).";
    }

    ostringstream oss;
    oss << "{\n";
    oss << "  \"success\": " << (success ? "true" : "false") << ",\n";
    oss << "  \"pid\": " << pid << ",\n";
    oss << "  \"size\": " << size << ",\n";
    oss << "  \"algorithm\": \"" << algorithm << "\",\n";
    oss << "  \"startAddress\": " << startAddr << ",\n";
    oss << "  \"explanation\": \"" << reason << "\"\n";
    oss << "}";

    return oss.str();
}

string ApiHandler::handleFreeMemory(const string& body) {
    int pid = extractIntField(body, "pid", 0);
    memoryManager.freeMemory(pid);
    return "{\"success\": true, \"message\": \"Memory released for process " + to_string(pid) + " and adjacent free blocks coalesced.\"}";
}

string ApiHandler::handleResetMemory() {
    memoryManager.resetMemory();
    return "{\"success\": true, \"message\": \"Memory reset to 1024 MB free\"}";
}

string ApiHandler::handleRunSchedule(const string& body) {
    string algo = extractStringField(body, "algorithm", "FCFS");
    int quantum = extractIntField(body, "quantum", 2);

    vector<Process> processes = processManager.getProcesses();
    SimulationResult res;

    if (algo == "FCFS") {
        res = SimulationEngine::runFCFS(processes);
    } else if (algo == "SJF") {
        res = SimulationEngine::runSJF(processes);
    } else if (algo == "Priority" || algo == "PriorityScheduling") {
        res = SimulationEngine::runPriority(processes);
    } else if (algo == "RoundRobin" || algo == "Round Robin") {
        res = SimulationEngine::runRoundRobin(processes, quantum);
    } else {
        res = SimulationEngine::runFCFS(processes);
    }

    return res.toJson();
}

