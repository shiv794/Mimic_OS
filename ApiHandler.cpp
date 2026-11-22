#include "ApiHandler.h"
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

// Robust JSON value extractors
static string escapeJsonString(const string& s) {
    ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else o << c;
    }
    return o.str();
}

static string extractStringField(const string& json, const string& key, const string& defaultVal = "") {
    string searchKey = "\"" + key + "\"";
    size_t pos = json.find(searchKey);
    if (pos == string::npos) return defaultVal;

    size_t colon = json.find(':', pos + searchKey.length());
    if (colon == string::npos) return defaultVal;

    size_t start = json.find('"', colon + 1);
    if (start == string::npos) return defaultVal;

    size_t end = start + 1;
    while (end < json.length()) {
        if (json[end] == '"' && json[end - 1] != '\\') {
            break;
        }
        end++;
    }
    if (end >= json.length()) return defaultVal;

    string val = json.substr(start + 1, end - start - 1);
    string unescaped = "";
    for (size_t i = 0; i < val.length(); i++) {
        if (val[i] == '\\' && i + 1 < val.length()) {
            if (val[i + 1] == 'n') { unescaped += '\n'; i++; }
            else if (val[i + 1] == 'r') { unescaped += '\r'; i++; }
            else if (val[i + 1] == 't') { unescaped += '\t'; i++; }
            else if (val[i + 1] == '"') { unescaped += '"'; i++; }
            else if (val[i + 1] == '\\') { unescaped += '\\'; i++; }
            else { unescaped += val[i + 1]; i++; }
        } else {
            unescaped += val[i];
        }
    }
    return unescaped;
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
      scheduler(&processManager, &cpu),
      shell(&processManager, &cpu, &scheduler, &memoryManager, &fileManager, &deadlockManager)
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
        } else if (path == "/api/files") {
            responseBody = handleGetFiles();
        } else if (path == "/api/deadlock") {
            responseBody = handleGetDeadlock();
        } else {
            statusCode = 404;
            responseBody = "{\"error\": \"Endpoint not found\"}";
        }
    } else if (method == "POST") {
        // Process Management
        if (path == "/api/processes/create") {
            responseBody = handleCreateProcess(requestBody);
        } else if (path == "/api/processes/delete") {
            responseBody = handleDeleteProcess(requestBody);
        } else if (path == "/api/processes/state") {
            responseBody = handleChangeState(requestBody);
        } else if (path == "/api/processes/reset") {
            responseBody = handleResetProcesses();
        }
        // CPU
        else if (path == "/api/cpu/execute") {
            responseBody = handleExecuteCPU(requestBody);
        } else if (path == "/api/cpu/stop") {
            responseBody = handleStopCPU();
        }
        // Memory Management
        else if (path == "/api/memory/allocate") {
            responseBody = handleAllocateMemory(requestBody);
        } else if (path == "/api/memory/free") {
            responseBody = handleFreeMemory(requestBody);
        } else if (path == "/api/memory/reset") {
            responseBody = handleResetMemory(requestBody);
        }
        // CPU Scheduling Simulation
        else if (path == "/api/schedule/run") {
            responseBody = handleRunSchedule(requestBody);
        }
        // File Management
        else if (path == "/api/files/create") {
            responseBody = handleCreateFile(requestBody);
        } else if (path == "/api/files/delete") {
            responseBody = handleDeleteFile(requestBody);
        } else if (path == "/api/files/write") {
            responseBody = handleWriteFile(requestBody);
        } else if (path == "/api/files/read") {
            responseBody = handleReadFile(requestBody);
        } else if (path == "/api/files/reset") {
            responseBody = handleResetFiles();
        } else if (path == "/api/directories/create") {
            responseBody = handleCreateDirectory(requestBody);
        } else if (path == "/api/directories/delete") {
            responseBody = handleDeleteDirectory(requestBody);
        }
        // Deadlock Detection
        else if (path == "/api/deadlock/detect") {
            responseBody = handleDetectDeadlock();
        } else if (path == "/api/deadlock/scenario") {
            responseBody = handleDeadlockScenario(requestBody);
        } else if (path == "/api/deadlock/update") {
            responseBody = handleDeadlockUpdate(requestBody);
        }
        // Shell Terminal
        else if (path == "/api/shell/execute") {
            responseBody = handleShellExecute(requestBody);
        }
        else {
            statusCode = 404;
            responseBody = "{\"error\": \"Endpoint not found\"}";
        }
    } else {
        statusCode = 405;
        responseBody = "{\"error\": \"Method not allowed\"}";
    }
}

// =========================================================================
// 1. DASHBOARD TELEMETRY & SYSTEM STATUS
// =========================================================================

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

    // Deadlock detection evaluation
    DeadlockResult dlRes = deadlockManager.evaluateDeadlock();

    ostringstream oss;
    oss << "{\n";
    oss << "  \"system\": \"Mimic OS\",\n";
    oss << "  \"status\": \"online\",\n";
    oss << "  \"version\": \"2.0.0\",\n";
    oss << "  \"cpu\": {\n";
    oss << "    \"busy\": " << (cpu.isBusy() ? "true" : "false") << ",\n";
    if (cpu.getCurrentProcess() != nullptr) {
        oss << "    \"currentPid\": " << cpu.getCurrentProcess()->getPID() << ",\n";
        oss << "    \"currentName\": \"" << escapeJsonString(cpu.getCurrentProcess()->getName()) << "\",\n";
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
    oss << "  },\n";
    oss << "  \"files\": {\n";
    oss << "    \"count\": " << fileManager.getFiles().size() << ",\n";
    oss << "    \"dirCount\": " << fileManager.getDirectories().size() << "\n";
    oss << "  },\n";
    oss << "  \"deadlock\": {\n";
    oss << "    \"isDeadlocked\": " << (dlRes.isDeadlocked ? "true" : "false") << ",\n";
    oss << "    \"status\": \"" << (dlRes.isDeadlocked ? "Deadlock Detected" : "No Deadlock (Safe)") << "\",\n";
    oss << "    \"deadlockedCount\": " << dlRes.deadlockedProcesses.size() << "\n";
    oss << "  }\n";
    oss << "}";

    return oss.str();
}

// =========================================================================
// 2. PROCESS MANAGEMENT ENDPOINTS
// =========================================================================

string ApiHandler::handleGetProcesses() {
    const vector<Process>& procList = processManager.getProcesses();
    const vector<PCB>& pcbList = processManager.getPCBs();

    ostringstream oss;
    oss << "{\n";
    oss << "  \"processes\": [\n";
    for (size_t i = 0; i < procList.size(); i++) {
        const Process& p = procList[i];
        oss << "    {\"pid\": " << p.getPID()
            << ", \"name\": \"" << escapeJsonString(p.getName()) << "\""
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
    // Keep deadlock manager in sync with process manager
    deadlockManager.syncWithProcesses(processManager.getProcesses());

    return "{\"success\": true, \"message\": \"Process created\", \"pid\": " + to_string(pid) + "}";
}

string ApiHandler::handleDeleteProcess(const string& body) {
    int pid = extractIntField(body, "pid", 0);
    bool deleted = processManager.deleteProcess(pid);
    // Keep deadlock manager in sync
    if (deleted) deadlockManager.syncWithProcesses(processManager.getProcesses());
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
    deadlockManager.syncWithProcesses(processManager.getProcesses());
    return "{\"success\": true, \"message\": \"Reset to default processes P1, P2, P3\"}";
}

// =========================================================================
// 3. CPU ENDPOINTS
// =========================================================================

string ApiHandler::handleGetCPU() {
    ostringstream oss;
    oss << "{\n";
    oss << "  \"busy\": " << (cpu.isBusy() ? "true" : "false") << ",\n";
    if (cpu.getCurrentProcess() != nullptr) {
        oss << "  \"currentProcess\": {\n";
        oss << "    \"pid\": " << cpu.getCurrentProcess()->getPID() << ",\n";
        oss << "    \"name\": \"" << escapeJsonString(cpu.getCurrentProcess()->getName()) << "\",\n";
        oss << "    \"burstTime\": " << cpu.getCurrentProcess()->getBurstTime() << ",\n";
        oss << "    \"priority\": " << cpu.getCurrentProcess()->getPriority() << ",\n";
        oss << "    \"state\": \"" << cpu.getCurrentProcess()->getState() << "\"\n";
        oss << "  },\n";
    } else {
        oss << "  \"currentProcess\": null,\n";
    }
    oss << "  \"lastProcessID\": " << cpu.getLastProcessID() << ",\n";
    oss << "  \"lastProcessName\": \"" << escapeJsonString(cpu.getLastProcessName()) << "\",\n";
    oss << "  \"lastActivity\": \"" << escapeJsonString(cpu.getLastActivity()) << "\"\n";
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
    return "{\"success\": true, \"message\": \"Executing process " + escapeJsonString(p->getName()) + "\"}";
}

string ApiHandler::handleStopCPU() {
    if (cpu.isBusy() && cpu.getCurrentProcess() != nullptr) {
        processManager.changeState(cpu.getCurrentProcess()->getPID(), "Ready");
    }
    cpu.stopProcess();
    return "{\"success\": true, \"message\": \"CPU stopped\"}";
}

// =========================================================================
// 4. MEMORY MANAGEMENT ENDPOINTS
// =========================================================================

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

    if (memoryManager.isProcessAllocated(pid)) {
        return "{\"success\": false, \"message\": \"Process P" + to_string(pid) + " already has memory allocated. Free it before allocating again.\", \"explanation\": \"Process P" + to_string(pid) + " already holds an active memory allocation. Please free it first.\"}";
    }

    vector<MemoryBlock> beforeBlocks = memoryManager.getMemoryBlocks();
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
    oss << "  \"algorithm\": \"" << escapeJsonString(algorithm) << "\",\n";
    oss << "  \"startAddress\": " << startAddr << ",\n";
    oss << "  \"explanation\": \"" << escapeJsonString(reason) << "\"\n";
    oss << "}";

    return oss.str();
}

string ApiHandler::handleFreeMemory(const string& body) {
    int pid = extractIntField(body, "pid", 0);
    memoryManager.freeMemory(pid);
    return "{\"success\": true, \"message\": \"Memory released for process " + to_string(pid) + " and adjacent free blocks coalesced.\"}";
}

string ApiHandler::handleResetMemory(const string& body) {
    int totalSize = extractIntField(body, "totalSize", -1);
    if (totalSize <= 0) {
        totalSize = extractIntField(body, "size", -1);
    }
    if (totalSize > 0) {
        memoryManager.resetMemory(totalSize);
        return "{\"success\": true, \"message\": \"Memory resized and reset to " + to_string(totalSize) + " MB free.\", \"totalMemory\": " + to_string(totalSize) + "}";
    } else {
        memoryManager.resetMemory();
        return "{\"success\": true, \"message\": \"Memory reset to " + to_string(memoryManager.getTotalMemory()) + " MB free.\", \"totalMemory\": " + to_string(memoryManager.getTotalMemory()) + "}";
    }
}

// =========================================================================
// 5. SCHEDULING SIMULATION ENDPOINTS
// =========================================================================

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

// =========================================================================
// 6. FILE MANAGEMENT ENDPOINTS
// =========================================================================

string ApiHandler::handleGetFiles() {
    const vector<File>& files = fileManager.getFiles();
    const vector<Directory>& dirs = fileManager.getDirectories();

    ostringstream oss;
    oss << "{\n";
    oss << "  \"totalFiles\": " << files.size() << ",\n";
    oss << "  \"totalDirectories\": " << dirs.size() << ",\n";
    oss << "  \"directories\": [\n";
    for (size_t i = 0; i < dirs.size(); i++) {
        oss << "    {\"name\": \"" << escapeJsonString(dirs[i].name) << "\"}";
        if (i + 1 < dirs.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    oss << "  \"files\": [\n";
    for (size_t i = 0; i < files.size(); i++) {
        oss << "    {\n";
        oss << "      \"name\": \"" << escapeJsonString(files[i].name) << "\",\n";
        oss << "      \"directory\": \"" << escapeJsonString(files[i].directory) << "\",\n";
        oss << "      \"size\": " << files[i].size << ",\n";
        oss << "      \"content\": \"" << escapeJsonString(files[i].content) << "\"\n";
        oss << "    }";
        if (i + 1 < files.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ]\n";
    oss << "}";

    return oss.str();
}

string ApiHandler::handleCreateFile(const string& body) {
    string name = extractStringField(body, "name", "");
    string dir = extractStringField(body, "directory", "/");
    if (dir.empty()) dir = "/";

    if (name.empty()) {
        return "{\"success\": false, \"message\": \"File name cannot be empty.\"}";
    }
    if (fileManager.fileExists(name, dir)) {
        return "{\"success\": false, \"message\": \"File '" + escapeJsonString(name) + "' already exists in " + escapeJsonString(dir) + ".\"}";
    }

    fileManager.createFile(name, dir);
    return "{\"success\": true, \"message\": \"File '" + escapeJsonString(name) + "' created successfully in " + escapeJsonString(dir) + ".\"}";
}

string ApiHandler::handleDeleteFile(const string& body) {
    string name = extractStringField(body, "name", "");
    string dir = extractStringField(body, "directory", "");

    if (name.empty() || !fileManager.fileExists(name, dir)) {
        return "{\"success\": false, \"message\": \"File '" + escapeJsonString(name) + "' not found.\"}";
    }

    fileManager.deleteFile(name, dir);
    return "{\"success\": true, \"message\": \"File '" + escapeJsonString(name) + "' deleted successfully.\"}";
}

string ApiHandler::handleWriteFile(const string& body) {
    string name = extractStringField(body, "name", "");
    string content = extractStringField(body, "content", "");
    string dir = extractStringField(body, "directory", "");

    if (name.empty()) {
        return "{\"success\": false, \"message\": \"File name cannot be empty.\"}";
    }

    fileManager.writeFile(name, content, dir);
    return "{\"success\": true, \"message\": \"Content written to '" + escapeJsonString(name) + "' (" + to_string(content.length()) + " bytes).\"}";
}

string ApiHandler::handleReadFile(const string& body) {
    string name = extractStringField(body, "name", "");
    string dir = extractStringField(body, "directory", "");
    const File* f = fileManager.getFile(name, dir);
    if (!f) {
        return "{\"success\": false, \"message\": \"File '" + escapeJsonString(name) + "' not found.\"}";
    }

    ostringstream oss;
    oss << "{\n";
    oss << "  \"success\": true,\n";
    oss << "  \"name\": \"" << escapeJsonString(f->name) << "\",\n";
    oss << "  \"directory\": \"" << escapeJsonString(f->directory) << "\",\n";
    oss << "  \"size\": " << f->size << ",\n";
    oss << "  \"content\": \"" << escapeJsonString(f->content) << "\"\n";
    oss << "}";
    return oss.str();
}

string ApiHandler::handleCreateDirectory(const string& body) {
    string name = extractStringField(body, "name", "");
    if (name.empty()) {
        return "{\"success\": false, \"message\": \"Directory name cannot be empty.\"}";
    }
    if (fileManager.directoryExists(name)) {
        return "{\"success\": false, \"message\": \"Directory '" + escapeJsonString(name) + "' already exists.\"}";
    }

    fileManager.createDirectory(name);
    return "{\"success\": true, \"message\": \"Directory '" + escapeJsonString(name) + "' created successfully.\"}";
}

string ApiHandler::handleDeleteDirectory(const string& body) {
    string name = extractStringField(body, "name", "");
    if (name == "/" || name.empty()) {
        return "{\"success\": false, \"message\": \"Cannot delete root directory or empty name.\"}";
    }
    if (!fileManager.directoryExists(name)) {
        return "{\"success\": false, \"message\": \"Directory '" + escapeJsonString(name) + "' not found.\"}";
    }

    fileManager.deleteDirectory(name);
    return "{\"success\": true, \"message\": \"Directory '" + escapeJsonString(name) + "' deleted successfully.\"}";
}

string ApiHandler::handleResetFiles() {
    fileManager.reset();
    return "{\"success\": true, \"message\": \"Virtual File System reset to default state.\"}";
}

// =========================================================================
// 7. DEADLOCK DETECTION ENDPOINTS
// =========================================================================

string ApiHandler::handleGetDeadlock() {
    // Always sync deadlock manager with current processes in ProcessManager
    deadlockManager.syncWithProcesses(processManager.getProcesses());

    int pCount = deadlockManager.getProcessCount();
    int rCount = deadlockManager.getResourceCount();
    const auto& alloc = deadlockManager.getAllocation();
    const auto& req = deadlockManager.getRequest();
    const auto& avail = deadlockManager.getAvailable();
    const auto& pids = deadlockManager.getProcessIds();
    const auto& pnames = deadlockManager.getProcessNames();

    DeadlockResult res = deadlockManager.evaluateDeadlock();

    ostringstream oss;
    oss << "{\n";
    oss << "  \"processCount\": " << pCount << ",\n";
    oss << "  \"resourceCount\": " << rCount << ",\n";
    oss << "  \"isDeadlocked\": " << (res.isDeadlocked ? "true" : "false") << ",\n";
    oss << "  \"status\": \"" << (res.isDeadlocked ? "DEADLOCK DETECTED" : "NO DEADLOCK (SAFE)") << "\",\n";

    // Process IDs array (actual PIDs, not row indices)
    oss << "  \"processIds\": [";
    for (size_t i = 0; i < pids.size(); i++) {
        oss << pids[i];
        if (i + 1 < pids.size()) oss << ",";
    }
    oss << "],\n";

    // Process Names array
    oss << "  \"processNames\": [";
    for (size_t i = 0; i < pnames.size(); i++) {
        oss << "\"" << escapeJsonString(pnames[i]) << "\"";
        if (i + 1 < pnames.size()) oss << ",";
    }
    oss << "],\n";

    // Deadlocked process ROW INDICES (for matrix highlighting)
    oss << "  \"deadlockedProcesses\": [";
    for (size_t i = 0; i < res.deadlockedProcesses.size(); i++) {
        oss << res.deadlockedProcesses[i];
        if (i + 1 < res.deadlockedProcesses.size()) oss << ",";
    }
    oss << "],\n";

    // Deadlocked PIDs (for readable display)
    oss << "  \"deadlockedPids\": [";
    for (size_t i = 0; i < res.deadlockedPids.size(); i++) {
        oss << res.deadlockedPids[i];
        if (i + 1 < res.deadlockedPids.size()) oss << ",";
    }
    oss << "],\n";

    // Safe sequence names
    oss << "  \"safeSequenceNames\": [";
    for (size_t i = 0; i < res.safeSequenceNames.size(); i++) {
        oss << "\"" << escapeJsonString(res.safeSequenceNames[i]) << "\"";
        if (i + 1 < res.safeSequenceNames.size()) oss << ",";
    }
    oss << "],\n";

    // Safe sequence row indices (legacy)
    oss << "  \"safeSequence\": [";
    for (size_t i = 0; i < res.safeSequence.size(); i++) {
        oss << res.safeSequence[i];
        if (i + 1 < res.safeSequence.size()) oss << ",";
    }
    oss << "],\n";

    // Available Vector
    oss << "  \"available\": [";
    for (size_t i = 0; i < avail.size(); i++) {
        oss << avail[i];
        if (i + 1 < avail.size()) oss << ",";
    }
    oss << "],\n";

    // Allocation Matrix
    oss << "  \"allocation\": [\n";
    for (size_t i = 0; i < alloc.size(); i++) {
        oss << "    [";
        for (size_t j = 0; j < alloc[i].size(); j++) {
            oss << alloc[i][j];
            if (j + 1 < alloc[i].size()) oss << ",";
        }
        oss << "]";
        if (i + 1 < alloc.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ],\n";

    // Request Matrix
    oss << "  \"request\": [\n";
    for (size_t i = 0; i < req.size(); i++) {
        oss << "    [";
        for (size_t j = 0; j < req[i].size(); j++) {
            oss << req[i][j];
            if (j + 1 < req[i].size()) oss << ",";
        }
        oss << "]";
        if (i + 1 < req.size()) oss << ",";
        oss << "\n";
    }
    oss << "  ]\n";
    oss << "}";

    return oss.str();
}

string ApiHandler::handleDetectDeadlock() {
    return handleGetDeadlock();
}

string ApiHandler::handleDeadlockScenario(const string& body) {
    string scenario = extractStringField(body, "scenario", "safe");
    if (scenario == "deadlock") {
        deadlockManager.loadDeadlockScenario();
    } else {
        deadlockManager.loadSafeScenario();
    }
    return handleGetDeadlock();
}

string ApiHandler::handleDeadlockUpdate(const string& body) {
    string type = extractStringField(body, "type", "allocation");
    int process = extractIntField(body, "process", 0);
    int resource = extractIntField(body, "resource", 0);
    int value = extractIntField(body, "value", 0);

    if (type == "available") {
        deadlockManager.setAvailable(resource, value);
    } else if (type == "request") {
        deadlockManager.setRequest(process, resource, value);
    } else {
        deadlockManager.setAllocation(process, resource, value);
    }

    return handleGetDeadlock();
}

// =========================================================================
// 8. CUSTOM MIMIC OS SHELL TERMINAL ENDPOINT
// =========================================================================

string ApiHandler::handleShellExecute(const string& body) {
    string cmd = extractStringField(body, "command", "");

    if (cmd.empty()) {
        return "{\"command\": \"\", \"output\": \"\", \"isClear\": false}";
    }

    string output = shell.executeCommandString(cmd);
    bool isClear = (output == "__CLEAR__");

    ostringstream oss;
    oss << "{\n";
    oss << "  \"command\": \"" << escapeJsonString(cmd) << "\",\n";
    oss << "  \"output\": \"" << (isClear ? "" : escapeJsonString(output)) << "\",\n";
    oss << "  \"isClear\": " << (isClear ? "true" : "false") << "\n";
    oss << "}";

    return oss.str();
}
