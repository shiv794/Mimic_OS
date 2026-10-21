#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <vector>
#include <string>

using namespace std;

struct MemoryBlock {
    int startAddress;
    int size;
    int pid;
    bool allocated;
};

class MemoryManager {
private:
    vector<MemoryBlock> memoryBlocks;
    int totalMemory;

public:
    MemoryManager(int memorySize);

    void firstFit(int pid, int size);
    void bestFit(int pid, int size);
    void worstFit(int pid, int size);

    void freeMemory(int pid);

    void displayMemory() const;

    // GUI / Telemetry Getters
    const vector<MemoryBlock>& getMemoryBlocks() const { return memoryBlocks; }
    int getTotalMemory() const { return totalMemory; }
    int getUsedMemory() const;
    int getFreeMemory() const;
    void resetMemory();
};

#endif
