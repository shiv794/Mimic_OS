/**
 * Mimic OS - Dashboard Controller
 */
const Dashboard = {
  async refresh() {
    try {
      const data = await API.getStatus();
      this.render(data);
    } catch (err) {
      // Server offline fallback
      this.renderOffline();
    }
  },

  render(data) {
    // 1. CPU Telemetry
    const cpuBadge = document.getElementById('dash-cpu-badge');
    const cpuTitle = document.getElementById('dash-cpu-title');
    const cpuSub = document.getElementById('dash-cpu-sub');
    const cpuLast = document.getElementById('dash-cpu-last');
    const tbCpu = document.getElementById('val-cpu');
    const dotCpu = document.getElementById('dot-cpu');

    if (data.cpu.busy) {
      cpuBadge.className = 'badge badge-accent';
      cpuBadge.textContent = 'RUNNING';
      cpuTitle.textContent = `RUNNING P${data.cpu.currentPid}`;
      cpuTitle.style.color = 'var(--state-running)';
      cpuSub.textContent = `Process "${data.cpu.currentName}" executing (${data.cpu.remainingBurst} units burst remaining)`;
      if (tbCpu) tbCpu.textContent = `RUNNING (P${data.cpu.currentPid})`;
      if (dotCpu) dotCpu.className = 'dot busy';
    } else {
      cpuBadge.className = 'badge';
      cpuBadge.textContent = 'IDLE';
      cpuTitle.textContent = 'CPU IDLE';
      cpuTitle.style.color = '#fff';
      cpuSub.textContent = 'No process currently dispatched to core';
      if (tbCpu) tbCpu.textContent = 'IDLE';
      if (dotCpu) dotCpu.className = 'dot online';
    }

    if (cpuLast) {
      cpuLast.textContent = data.cpu.lastProcessName || 'None';
    }

    // 2. Processes Telemetry
    const procTotal = document.getElementById('dash-proc-total');
    const countNew = document.getElementById('dash-count-new');
    const countReady = document.getElementById('dash-count-ready');
    const countRunning = document.getElementById('dash-count-running');
    const countWaiting = document.getElementById('dash-count-waiting');
    const countTerm = document.getElementById('dash-count-term');
    const tbProcCount = document.getElementById('val-proc-count');

    if (procTotal) procTotal.textContent = `${data.processes.total} Active`;
    if (countNew) countNew.textContent = data.processes.new;
    if (countReady) countReady.textContent = data.processes.ready;
    if (countRunning) countRunning.textContent = data.processes.running;
    if (countWaiting) countWaiting.textContent = data.processes.waiting;
    if (countTerm) countTerm.textContent = data.processes.terminated;
    if (tbProcCount) tbProcCount.textContent = data.processes.total;

    // 3. Memory Telemetry
    const memPercent = document.getElementById('dash-mem-percent');
    const memFill = document.getElementById('dash-mem-fill');
    const memUsed = document.getElementById('dash-mem-used');
    const memFree = document.getElementById('dash-mem-free');
    const tbMemSummary = document.getElementById('val-mem-summary');

    const pct = data.memory.usedPercent.toFixed(1);
    if (memPercent) memPercent.textContent = `${pct}% Used`;
    if (memFill) memFill.style.width = `${pct}%`;
    if (memUsed) memUsed.textContent = `${data.memory.used} MB`;
    if (memFree) memFree.textContent = `${data.memory.free} MB`;
    if (tbMemSummary) tbMemSummary.textContent = `${data.memory.used} / ${data.memory.total} MB`;
  },

  renderOffline() {
    const cpuTitle = document.getElementById('dash-cpu-title');
    if (cpuTitle) cpuTitle.textContent = 'CORE OFFLINE';
    const cpuSub = document.getElementById('dash-cpu-sub');
    if (cpuSub) cpuSub.textContent = 'Run mimic_server.exe in workspace to connect';
  }
};

