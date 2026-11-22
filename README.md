# Mimic OS — Educational Operating System Simulator
> **BTech Semester 3 Project-Based Learning (PBL)**  
> Built primarily in **C++** with a modern, interactive Developer/OS Frontend.

---

## 1. Project Overview

**Mimic OS** is an educational operating-system simulator designed to demonstrate fundamental OS concepts visually to students and teachers. Rather than operating as a real kernel, it provides a transparent, visual simulation of core OS subsystems:

1. **Process Management & PCB (Process Control Block)**: Process creation, state transitions (`New`, `Ready`, `Running`, `Waiting`, `Terminated`), priority levels, and PCB tracking.
2. **CPU Execution Core**: CPU die and socket visualization, program counters, execution registers, burst countdown, and active/idle state transitions.
3. **CPU Scheduling Simulator**: Real-time simulation of four core scheduling algorithms implemented in C++:
   - **FCFS** (First-Come, First-Served)
   - **SJF** (Shortest Job First — Non-preemptive)
   - **Priority Scheduling** (Non-preemptive, lower number = higher priority)
   - **Round Robin** (Preemptive with configurable time quantum and circular queue visualization)
   - **Dynamic Gantt Chart**: Generated dynamically from C++ simulation intervals.
   - **Ready Queue & Step-by-Step Playback**: Step forward/backward with textbook narration for classroom demonstrations.
   - **Calculated OS Metrics**: Completion Time ($CT$), Turnaround Time ($TAT = CT - AT$), Waiting Time ($WT = TAT - BT$), Response Time ($RT$), and summary averages.
4. **Memory Management Subsystem**: Contiguous physical RAM partitioning (1024 MB):
   - **First Fit**: Allocates the first free hole with $\text{size} \ge \text{requested}$.
   - **Best Fit**: Allocates the free hole leaving the smallest remaining fragment.
   - **Worst Fit**: Allocates the largest available free hole.
   - **Decision Explainer**: Explains *why* a particular block was chosen by the C++ engine.
   - **Hole Coalescing**: Automatic merging of adjacent free memory partitions upon deallocation.
5. **Virtual File System & File Management**: Simulated OS file explorer:
   - Create, write, read, and delete virtual files.
   - Create, list, and delete virtual directories.
   - Live interactive text editor and file inspector.
6. **Deadlock Detection Subsystem**: Resource allocation & request analysis:
   - Allocation Matrix ($P \times R$), Request Matrix ($P \times R$), and Available Vector ($R$).
   - Banker's / Cycle Detection algorithm running directly on C++ `DeadlockManager`.
   - Test scenarios: one-click loading of safe vs deadlocked states.
   - Process & resource node state visualizer distinguishing safe, waiting, and deadlocked processes.
7. **Custom Mimic OS Terminal**: Dedicated interactive console:
   - Native OS shell communicating directly with the shared C++ `Shell` class.
   - Command history navigation (Up/Down arrow keys).
   - Quick command suggestion pills (`help`, `ps`, `ls`, `memory`, `deadlock`, `rr 2`, etc.).
   - Full support for kernel commands across all subsystems.
8. **Integrated OS Pipeline**: Holistic cross-module view connecting `Process Pool ➜ Ready Queue ➜ CPU Core ➜ Memory Allocation ➜ Deadlock Monitor ➜ Terminated`.

---

## 2. Architecture & Design

Mimic OS uses a unified, zero-dependency integration architecture sharing the **exact same C++ manager instances**:

```
┌─────────────────────────────────────────────────────────────────────────┐
│                          FRONTEND / GUI                                 │
│       (HTML5 • CSS3 Dark OS Theme • Modular ES6 JavaScript)            │
│  - Dashboard          - Process Management    - CPU Hardware Core       │
│  - CPU Scheduling     - Memory Management     - Virtual File System     │
│  - Deadlock Detection - Mimic OS Terminal     - Integrated Flow         │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ HTTP REST API (JSON)
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                C++ WINSOCK BRIDGE LAYER (HttpServer)                    │
│   - Embedded zero-dependency HTTP/1.1 server (Winsock2)                 │
│   - Endpoints: /api/status, /api/processes, /api/cpu, /api/memory,      │
│     /api/schedule/run, /api/files, /api/deadlock, /api/shell/execute    │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ Direct C++ Invocations
                                     ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                      C++ SIMULATION ENGINE                              │
│   - ProcessManager.cpp  - PCB.cpp             - CPU.cpp                 │
│   - Scheduler.cpp       - MemoryManager.cpp   - FileManager.cpp         │
│   - DeadlockManager.cpp - Shell.cpp           - SimulationEngine.cpp    │
└─────────────────────────────────────────────────────────────────────────┘
```

- **Shared State**: A process created via GUI is visible in the terminal (`ps`). A file created in the terminal (`touch test.txt`) is instantly visible in the Virtual File System explorer.
- **No external dependencies**: Runs natively on Windows using standard MinGW GCC (`g++ -std=c++11 -lws2_32`). No Node.js, Python, or npm required.

---

## 3. How to Run

### Option A: 1-Click Launch (Recommended)
Double-click `run_gui.bat` or run in PowerShell:
```powershell
.\run_gui.ps1
```
*(Or in Command Prompt: `run_gui.bat`)*

This compiles the project and opens your default browser automatically to:
```
http://localhost:8080
```

### Option B: Dedicated GUI Server via PowerShell
```powershell
g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp FileManager.cpp DeadlockManager.cpp Shell.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp server_main.cpp -o mimic_server.exe -lws2_32
.\mimic_server.exe 8080
```

### Option C: Classic Console CLI Menu (Terminal Mode)
```powershell
g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp FileManager.cpp DeadlockManager.cpp Shell.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp main.cpp -o mimic.exe -lws2_32
.\mimic.exe
```
Inside `mimic.exe`, option `9` launches the interactive Mimic OS Shell, and option `10` launches the Interactive Web GUI.

---

## 4. Subsystem Guide

### 1. Process Management (`Process.cpp`, `PCB.cpp`, `ProcessManager.cpp`)
- **Process Table**: Shows PID, Process Name, Arrival Time, Burst Time, Priority, and State with color-coded chips (`New`, `Ready`, `Running`, `Waiting`, `Terminated`).
- **PCB Table**: Displays the internal Process Control Block registers and Program Counters.
- **Controls**: Interactive creation form, state change dropdown, terminate, and delete.

### 2. CPU Core (`CPU.cpp`)
- **CPU Die & Socket Visualizer**: Hardware styling with pin traces, clock indicator, and pulse glow when busy.
- **Status**: Clearly shows `IDLE` vs `RUNNING (Px)`.
- **Registers**: Displays PID Register, Program Counter, Status Register, and Last Executed process.
- **Manual Controls**: Dispatch ready processes directly to CPU or halt core.

### 3. CPU Scheduling (`Scheduler.cpp`, `SimulationEngine.cpp`)
- **Algorithms**: FCFS, SJF, Priority, and Round Robin (with configurable time quantum).
- **Dynamic Gantt Chart**: Automatically mapped from backend intervals.
- **Ready Queue Conveyor**: Live visualization of queue order; circular queue loop arrow for Round Robin preemption.
- **Simulation Controls**: Play, Pause, Resume, Reset, and **Step** (single-step execution with narrative step description).
- **Performance Metrics**: Calculates Completion Time ($CT$), Turnaround Time ($TAT$), Waiting Time ($WT$), Response Time ($RT$), and summary averages.

### 4. Memory Management (`MemoryManager.cpp`)
- **Physical Memory Bar**: 1024 MB RAM visual map with contiguous block partitions.
- **Strategies**: First Fit, Best Fit, and Worst Fit.
- **Explainer**: Provides an educational breakdown of *why* a particular block was chosen.
- **Hole Coalescing**: Releases memory and automatically coalesces adjacent free partitions.

### 5. Virtual File System (`FileManager.cpp`)
- **Directory Tree**: Browse directories (`/`, `/bin`, `/docs`, `/home`), create directories, and delete non-root directories.
- **File Explorer**: Create files, view file sizes, delete files.
- **Built-in Editor**: Select any file to inspect and edit content in real time. Changes are written directly to C++ `FileManager`.

### 6. Deadlock Detection (`DeadlockManager.cpp`)
- **Live Matrices**: Displays Allocation Matrix, Request Matrix, and Available Vector.
- **Interactive Scenarios**: Load a textbook Safe scenario or Deadlock scenario with one click.
- **Detection Algorithm**: Executes banker's style graph reduction and highlights deadlocked processes or safe execution sequence ($P0 \to P2 \to \dots$).
- **Cell Adjustments**: Edit matrix values directly in the UI to experiment with resource configurations.

### 7. Custom Mimic OS Terminal (`Shell.cpp`)
- **Terminal Console**: Dark terminal window with custom scanline feel and command history.
- **Kernel Bridge**: Executes commands on the shared C++ state:
  - `ps`, `pcb`, `create_process <pid> <name> <arr> <burst> <pri>`, `kill <pid>`, `delete_process <pid>`
  - `cpu`
  - `fcfs`, `sjf`, `priority`, `rr <quantum>`
  - `memory`, `firstfit <pid> <size>`, `bestfit <pid> <size>`, `worstfit <pid> <size>`, `free <pid>`
  - `touch <file>`, `cat <file>`, `write <file> <content>`, `rm <file>`, `mkdir <dir>`, `rmdir <dir>`, `ls`, `dirs`
  - `deadlock`, `clear`, `help`, `exit`

### 8. Integrated OS Pipeline
- Cross-module inspector: Click any process (e.g. $P1$) to inspect its simultaneous footprint across the Process Table, CPU execution core, and Memory Manager.

---

## 5. Keyboard Shortcuts
- `1` : Switch to Dashboard
- `2` : Switch to Process Management
- `3` : Switch to CPU Core
- `4` : Switch to CPU Scheduling Simulator
- `5` : Switch to Memory Management
- `6` : Switch to Virtual File System
- `7` : Switch to Deadlock Detection
- `8` : Switch to Mimic OS Terminal
- `9` : Switch to Integrated OS Flow
- `Space` : Start / Pause Simulation playback
- `S` : Step forward in simulation
