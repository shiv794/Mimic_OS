#include "MemoryManager.h"
#include <iostream>

using namespace std;


// ======================================================
// CONSTRUCTOR
// ======================================================

MemoryManager::MemoryManager(int memorySize) {

    totalMemory = memorySize;

    MemoryBlock initialBlock;

    initialBlock.startAddress = 0;
    initialBlock.size = memorySize;
    initialBlock.pid = -1;
    initialBlock.allocated = false;

    memoryBlocks.push_back(initialBlock);
}


// ======================================================
// FIRST FIT
// ======================================================

void MemoryManager::firstFit(int pid, int size) {

    for (size_t i = 0; i < memoryBlocks.size(); i++) {

        if (!memoryBlocks[i].allocated &&
            memoryBlocks[i].size >= size) {

            int remaining =
                memoryBlocks[i].size - size;

            memoryBlocks[i].size = size;
            memoryBlocks[i].pid = pid;
            memoryBlocks[i].allocated = true;


            if (remaining > 0) {

                MemoryBlock newBlock;

                newBlock.startAddress =
                    memoryBlocks[i].startAddress + size;

                newBlock.size = remaining;

                newBlock.pid = -1;

                newBlock.allocated = false;


                memoryBlocks.insert(
                    memoryBlocks.begin() + i + 1,
                    newBlock);
            }


            cout << "\nFirst Fit: "
                 << size
                 << " MB allocated to P"
                 << pid << ".\n";

            return;
        }
    }


    cout << "\nFirst Fit: Not enough memory for P"
         << pid << ".\n";
}


// ======================================================
// BEST FIT
// ======================================================

void MemoryManager::bestFit(int pid, int size) {

    int bestIndex = -1;
    int smallestDifference = 999999;


    for (size_t i = 0; i < memoryBlocks.size(); i++) {

        if (!memoryBlocks[i].allocated &&
            memoryBlocks[i].size >= size) {

            int difference =
                memoryBlocks[i].size - size;


            if (difference < smallestDifference) {

                smallestDifference = difference;

                bestIndex = static_cast<int>(i);
            }
        }
    }


    if (bestIndex == -1) {

        cout << "\nBest Fit: Not enough memory for P"
             << pid << ".\n";

        return;
    }


    int remaining =
        memoryBlocks[bestIndex].size - size;


    memoryBlocks[bestIndex].size = size;
    memoryBlocks[bestIndex].pid = pid;
    memoryBlocks[bestIndex].allocated = true;


    if (remaining > 0) {

        MemoryBlock newBlock;

        newBlock.startAddress =
            memoryBlocks[bestIndex].startAddress + size;

        newBlock.size = remaining;

        newBlock.pid = -1;

        newBlock.allocated = false;


        memoryBlocks.insert(
            memoryBlocks.begin() + bestIndex + 1,
            newBlock);
    }


    cout << "\nBest Fit: "
         << size
         << " MB allocated to P"
         << pid << ".\n";
}


// ======================================================
// WORST FIT
// ======================================================

void MemoryManager::worstFit(int pid, int size) {

    int worstIndex = -1;
    int largestSize = -1;


    for (size_t i = 0; i < memoryBlocks.size(); i++) {

        if (!memoryBlocks[i].allocated &&
            memoryBlocks[i].size >= size) {

            if (memoryBlocks[i].size > largestSize) {

                largestSize =
                    memoryBlocks[i].size;

                worstIndex =
                    static_cast<int>(i);
            }
        }
    }


    if (worstIndex == -1) {

        cout << "\nWorst Fit: Not enough memory for P"
             << pid << ".\n";

        return;
    }


    int remaining =
        memoryBlocks[worstIndex].size - size;


    memoryBlocks[worstIndex].size = size;
    memoryBlocks[worstIndex].pid = pid;
    memoryBlocks[worstIndex].allocated = true;


    if (remaining > 0) {

        MemoryBlock newBlock;

        newBlock.startAddress =
            memoryBlocks[worstIndex].startAddress + size;

        newBlock.size = remaining;

        newBlock.pid = -1;

        newBlock.allocated = false;


        memoryBlocks.insert(
            memoryBlocks.begin() + worstIndex + 1,
            newBlock);
    }


    cout << "\nWorst Fit: "
         << size
         << " MB allocated to P"
         << pid << ".\n";
}


// ======================================================
// FREE MEMORY
// ======================================================

void MemoryManager::freeMemory(int pid) {

    bool found = false;


    for (MemoryBlock& block : memoryBlocks) {

        if (block.allocated &&
            block.pid == pid) {

            block.pid = -1;
            block.allocated = false;

            found = true;

            cout << "\nMemory released from P"
                 << pid << ".\n";
        }
    }


    if (!found) {

        cout << "\nNo memory allocation found for P"
             << pid << ".\n";

        return;
    }


    // Merge adjacent free blocks

    for (size_t i = 0;
         i + 1 < memoryBlocks.size();) {

        if (!memoryBlocks[i].allocated &&
            !memoryBlocks[i + 1].allocated) {

            memoryBlocks[i].size +=
                memoryBlocks[i + 1].size;

            memoryBlocks.erase(
                memoryBlocks.begin() + i + 1);
        }

        else {

            i++;
        }
    }
}


// ======================================================
// DISPLAY MEMORY
// ======================================================

void MemoryManager::displayMemory() const {

    cout << "\n\n";
    cout << "====================================================\n";
    cout << "                 MEMORY STATUS\n";
    cout << "====================================================\n";


    cout << "Total Memory: "
         << totalMemory
         << " MB\n\n";


    cout << "----------------------------------------------------\n";

    cout << "Start\tSize\tStatus\n";

    cout << "----------------------------------------------------\n";


    for (const MemoryBlock& block : memoryBlocks) {

        cout << block.startAddress
             << "\t"
             << block.size
             << " MB\t";


        if (block.allocated) {

            cout << "P"
                 << block.pid
                 << " Allocated";
        }

        else {

            cout << "FREE";
        }


        cout << endl;
    }


    cout << "----------------------------------------------------\n";
}


int MemoryManager::getUsedMemory() const {
    int used = 0;
    for (size_t i = 0; i < memoryBlocks.size(); i++) {
        if (memoryBlocks[i].allocated) {
            used += memoryBlocks[i].size;
        }
    }
    return used;
}

int MemoryManager::getFreeMemory() const {
    return totalMemory - getUsedMemory();
}

void MemoryManager::resetMemory() {
    memoryBlocks.clear();
    MemoryBlock initialBlock;
    initialBlock.startAddress = 0;
    initialBlock.size = totalMemory;
    initialBlock.pid = -1;
    initialBlock.allocated = false;
    memoryBlocks.push_back(initialBlock);
}
