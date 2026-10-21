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
5. **Integrated OS Pipeline**: Holistic cross-module view connecting `Process Pool ➜ Ready Queue ➜ CPU Core ➜ Memory Allocation ➜ Terminated`.

---

## 2. Architecture & Design

Mimic OS uses a clean, zero-dependency integration architecture:

```
┌─────────────────────────────────────────────────────────────┐
│                      FRONTEND / GUI                         │
│   (HTML5 • CSS3 Dark OS Theme • Modular ES6 JavaScript)    │
│  - Dashboard  - Process Management  - CPU Hardware Core     │
│  - CPU Scheduling & Gantt  - Memory Map  - Integrated Flow  │
└──────────────────────────────┬──────────────────────────────┘
                               │ HTTP REST API (JSON)
                               ▼
┌─────────────────────────────────────────────────────────────┐
│            C++ WINSOCK BRIDGE LAYER (HttpServer)            │
│   - Embedded zero-dependency HTTP/1.1 server (Winsock2)     │
│   - Endpoints: /api/status, /api/processes, /api/cpu,       │
│     /api/schedule/run, /api/memory/allocate, etc.           │
└──────────────────────────────┬──────────────────────────────┘
                               │ Direct C++ Class Invocations
                               ▼
┌─────────────────────────────────────────────────────────────┐
│                  C++ SIMULATION ENGINE                      │
│   - ProcessManager.cpp  - PCB.cpp       - CPU.cpp           │
│   - Scheduler.cpp       - MemoryManager.cpp                 │
│   - SimulationEngine.cpp                                    │
└─────────────────────────────────────────────────────────────┘
```

- **All OS logic runs in C++**: The frontend never fakes or duplicates scheduling algorithms or memory partitioning in JavaScript.
- **No external dependencies**: Runs natively on Windows using standard MinGW GCC (`g++ -std=c++11 -lws2_32`). No Node.js, Python, or npm required.

---

## 3. How to Run

### Option A: 1-Click Launch (Recommended for Presentations)
Double-click `run_gui.bat` or run:
```cmd
run_gui.bat
```
*(Or in PowerShell: `.\run_gui.ps1`)*

This compiles the project and opens your default browser automatically to:
```
http://localhost:8080
```

### Option B: Dedicated GUI Server
```cmd
g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp server_main.cpp -o mimic_server.exe -lws2_32
.\mimic_server.exe
```

### Option C: Classic Console CLI Menu (Original Terminal Mode)
```cmd
g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp main.cpp -o mimic.exe -lws2_32
.\mimic.exe
```
Inside `mimic.exe`, option `7` also allows launching the Interactive Web GUI.

---

## 4. Subsystem Guide

### 1. Process Management (`Process.cpp`, `PCB.cpp`, `ProcessManager.cpp`)
- **Process Table**: Shows PID, Process Name, Arrival Time, Burst Time, Priority, and State with distinct color chips (`New`, `Ready`, `Running`, `Waiting`, `Terminated`).
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
- **Performance Metrics**: Calculates Completion Time ($CT$), Turnaround Time ($TAT$), Waiting Time ($WT$), Response Time ($RT$), Average Turnaround Time, Average Waiting Time, Average Response Time, and CPU Utilization.

### 4. Memory Management (`MemoryManager.cpp`)
- **Physical Memory Bar**: 1024 MB RAM visual map with contiguous block partitions.
- **Strategies**: First Fit, Best Fit, and Worst Fit.
- **Explainer**: Provides an educational breakdown of *why* a particular block was chosen.
- **Hole Coalescing**: Releases memory and automatically coalesces adjacent free partitions.

### 5. Integrated OS Pipeline
- Cross-module inspector: Click any process (e.g. $P1$) to inspect its simultaneous footprint across the Process Table, CPU execution core, and Memory Manager.

---

## 5. Keyboard Shortcuts
- `1` : Switch to Dashboard
- `2` : Switch to Process Management
- `3` : Switch to CPU Core
- `4` : Switch to CPU Scheduling Simulator
- `5` : Switch to Memory Management
- `6` : Switch to Integrated OS Flow
- `Space` : Start / Pause Simulation
- `S` : Step forward in simulation

