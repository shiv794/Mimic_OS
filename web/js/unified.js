/**
 * Mimic OS - Integrated OS Pipeline & Resource Inspector
 * Connects Process Management -> Ready Queue -> CPU Core -> Memory Allocation -> Terminated
 */
const Unified = {
  selectedPid: null,

  init() {
    // Initialized when processes refresh
  },

  async inspectProcess(pid) {
    this.selectedPid = pid;
    App.navigateTo('unified');

    // Highlight chip
    document.querySelectorAll('.proc-chip-btn').forEach(b => b.classList.remove('active'));
    const activeChip = document.getElementById(`chip-proc-${pid}`);
    if (activeChip) activeChip.classList.add('active');

    // Fetch fresh process, cpu, and memory state
    try {
      const [procData, cpuData, memData] = await Promise.all([
        API.getProcesses(),
        API.getCPU(),
        API.getMemory()
      ]);

      const proc = (procData.processes || []).find(p => p.pid === pid);
      const pcb = (procData.pcbs || []).find(p => p.pid === pid);
      const isRunning = cpuData.busy && cpuData.currentProcess && cpuData.currentProcess.pid === pid;
      const memBlocks = (memData.blocks || []).filter(b => b.allocated && b.pid === pid);

      this.renderInspector(proc, pcb, isRunning, cpuData, memBlocks);
    } catch (err) {
      console.warn('[Unified] Inspection failed:', err.message);
    }
  },

  renderInspector(proc, pcb, isRunning, cpuData, memBlocks) {
    const pcbBox = document.getElementById('inspect-pcb-box');
    const cpuBox = document.getElementById('inspect-cpu-box');
    const memBox = document.getElementById('inspect-mem-box');

    if (!proc) {
      if (pcbBox) pcbBox.innerHTML = '<span style="color:var(--text-dim)">Process not found in kernel table.</span>';
      return;
    }

    // 1. Process & PCB Box
    const pcHex = pcb ? '0x' + pcb.programCounter.toString(16).padStart(4, '0').toUpperCase() : '0x0000';
    if (pcbBox) {
      pcbBox.innerHTML = `
        <div class="inspector-kv"><span class="k">Process ID:</span><span class="v">P${proc.pid}</span></div>
        <div class="inspector-kv"><span class="k">Process Name:</span><span class="v">${proc.name}</span></div>
        <div class="inspector-kv"><span class="k">Kernel State:</span><span class="v" style="color:var(--state-${proc.state.toLowerCase()})">${proc.state.toUpperCase()}</span></div>
        <div class="inspector-kv"><span class="k">Priority Level:</span><span class="v">${proc.priority}</span></div>
        <div class="inspector-kv"><span class="k">Arrival Time:</span><span class="v">${proc.arrivalTime} units</span></div>
        <div class="inspector-kv"><span class="k">Burst Time:</span><span class="v">${proc.burstTime} units</span></div>
        <div class="inspector-kv"><span class="k">Program Counter:</span><span class="v"><code>${pcHex}</code></span></div>
      `;
    }

    // 2. CPU Core Box
    if (cpuBox) {
      if (isRunning) {
        cpuBox.innerHTML = `
          <div style="background:var(--state-running-bg); border:1px solid var(--state-running); padding:0.6rem; border-radius:var(--radius-sm); margin-bottom:0.5rem; text-align:center;">
            <strong style="color:var(--state-running)">Currently Executing on Mimic Core 1</strong>
          </div>
          <div class="inspector-kv"><span class="k">Core State:</span><span class="v" style="color:var(--state-running)">ACTIVE RUNNING</span></div>
          <div class="inspector-kv"><span class="k">Remaining Burst:</span><span class="v">${proc.burstTime} units</span></div>
          <div class="inspector-kv"><span class="k">Time Quantum Slice:</span><span class="v">Active</span></div>
        `;
      } else {
        const isTerm = proc.state === 'Terminated';
        cpuBox.innerHTML = `
          <div class="inspector-kv"><span class="k">CPU Allocation:</span><span class="v">${isTerm ? 'Execution Completed' : 'Waiting in Ready Queue'}</span></div>
          <div class="inspector-kv"><span class="k">Last Core Activity:</span><span class="v">${cpuData.lastActivity || 'None'}</span></div>
          <div style="margin-top:0.85rem;">
            ${!isTerm ? `
              <button class="btn btn-primary btn-sm btn-block" onclick="CPU.handleDispatchSpecific(${proc.pid})">
                Dispatch P${proc.pid} to CPU Now
              </button>
            ` : '<span style="color:var(--text-dim); font-size:0.75rem;">Process has terminated</span>'}
          </div>
        `;
      }
    }

    // 3. Memory Box
    if (memBox) {
      if (memBlocks.length > 0) {
        const totalAllocated = memBlocks.reduce((acc, b) => acc + b.size, 0);
        memBox.innerHTML = `
          <div class="inspector-kv"><span class="k">Total RAM Occupied:</span><span class="v"><strong>${totalAllocated} MB</strong></span></div>
          ${memBlocks.map((b, idx) => `
            <div style="background:rgba(255,255,255,0.03); border:1px solid var(--border-subtle); padding:0.5rem; border-radius:var(--radius-sm); margin-top:0.4rem;">
              <div class="inspector-kv"><span class="k">Partition #${idx}:</span><span class="v">${b.size} MB</span></div>
              <div class="inspector-kv"><span class="k">Base Address:</span><span class="v"><code>0x${b.startAddress.toString(16).padStart(4, '0').toUpperCase()} (${b.startAddress} MB)</code></span></div>
            </div>
          `).join('')}
          <div style="margin-top:0.75rem;">
            <button class="btn btn-danger btn-sm btn-block" onclick="Memory.free(${proc.pid})">
              Release Allocated Memory
            </button>
          </div>
        `;
      } else {
        memBox.innerHTML = `
          <div style="color:var(--text-dim); margin-bottom:0.6rem;">No physical memory currently allocated to this process.</div>
          <button class="btn btn-outline btn-sm btn-block" onclick="App.navigateTo('memory')">
            Allocate Memory in MMU
          </button>
        `;
      }
    }
  }
};

// Dispatch helper for specific process
CPU.handleDispatchSpecific = async function(pid) {
  try {
    await API.executeCPU(pid);
    App.showToast(`P${pid} dispatched to CPU`, 'success');
    await CPU.refresh();
    await Processes.refresh();
    await Dashboard.refresh();
    Unified.inspectProcess(pid);
  } catch (err) {
    App.showToast('Dispatch failed', 'error');
  }
};

