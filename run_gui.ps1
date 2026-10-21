Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "                    MIMIC OS - LAUNCHER" -ForegroundColor Cyan
Write-Host "      BTech Semester 3 PBL Operating System Visual Simulator" -ForegroundColor White
Write-Host "======================================================================" -ForegroundColor Cyan

Write-Host "[1/2] Compiling Mimic OS with g++ (MinGW)..." -ForegroundColor Yellow
g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp server_main.cpp -o mimic_server.exe -lws2_32
if ($LASTEXITCODE -ne 0) {
    Write-Host "[ERROR] Compilation failed. Ensure g++ is in your PATH." -ForegroundColor Red
    exit $LASTEXITCODE
}

g++ -std=c++11 -Wall Process.cpp PCB.cpp ProcessManager.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp SimulationEngine.cpp ApiHandler.cpp HttpServer.cpp main.cpp -o mimic.exe -lws2_32

Write-Host "[2/2] Launching Mimic OS Web GUI..." -ForegroundColor Green
Start-Process ".\mimic_server.exe" -ArgumentList "8080"
Write-Host "Mimic OS is running on http://localhost:8080" -ForegroundColor Cyan

