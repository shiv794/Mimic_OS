@echo off
title Mimic OS - Educational Operating System Simulator
echo ======================================================================
echo                     MIMIC OS - LAUNCHER
echo       BTech Semester 3 PBL Operating System Visual Simulator
echo ======================================================================
echo [1/2] Compiling Mimic OS backend and GUI server with g++...
g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp server_main.cpp -o mimic_server.exe -lws2_32
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed. Ensure MinGW g++ is in your PATH.
    pause
    exit /b %ERRORLEVEL%
)

g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp main.cpp -o mimic.exe -lws2_32
echo [2/2] Launching Mimic OS Web GUI at http://localhost:8080...
start mimic_server.exe 8080
echo Mimic OS is running! You can also run mimic.exe in a terminal for CLI mode.

