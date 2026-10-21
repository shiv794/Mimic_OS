/**
 * Mimic OS - Process Management Controller
 */
const Processes = {
  currentProcesses: [],
  currentPCBs: [],

  init() {
    const form = document.getElementById('form-create-process');
    if (form) {
      form.addEventListener('submit', (e) => this.handleCreate(e));
    }

    const btnDefault = document.getElementById('btn-load-preset-default');
    if (btnDefault) {
      btnDefault.addEventListener('click', () => this.loadPreset('default'));
    }

    const btnMixed = document.getElementById('btn-load-preset-mixed');
    if (btnMixed) {
      btnMixed.addEventListener('click', () => this.loadPreset('mixed'));
    }

    // Tabs inside Process Management
    const tabBtns = document.querySelectorAll('.tabs-header .tab-btn');
    tabBtns.forEach(btn => {
      btn.addEventListener('click', () => {
        tabBtns.forEach(b => b.classList.remove('active'));
        document.querySelectorAll('.proc-tables-card .tab-content').forEach(c => c.classList.remove('active'));
        btn.classList.add('active');
        const target = document.getElementById(btn.dataset.tab);
        if (target) target.classList.add('active');
      });
    });
  },

  async refresh() {
    try {
      const data = await API.getProcesses();
      this.currentProcesses = data.processes || [];
      this.currentPCBs = data.pcbs || [];
      this.render();
      this.updateOtherModuleDropdowns();
    } catch (err) {
      console.warn('[Processes] Refresh failed:', err.message);
    }
  },

  render() {
    // 1. Process Table
    const tbodyProc = document.getElementById('tbody-processes');
    const badgeCount = document.getElementById('count-procs-badge');
    if (badgeCount) badgeCount.textContent = this.currentProcesses.length;

    if (tbodyProc) {
      if (this.currentProcesses.length === 0) {
        tbodyProc.innerHTML = `<tr><td colspan="8" style="text-align:center; color:var(--text-dim); padding:2rem;">No processes in table. Create one above.</td></tr>`;
      } else {
        tbodyProc.innerHTML = this.currentProcesses.map(p => {
          const stateLower = p.state.toLowerCase();
          const chipClass = `chip chip-${stateLower}`;

          return `
            <tr>
              <td><strong>P${p.pid}</strong></td>
              <td>${p.name}</td>
              <td><code>${p.arrivalTime}</code></td>
              <td><code>${p.burstTime}</code></td>
              <td><span class="badge ${p.priority === 1 ? 'badge-accent' : ''}">${p.priority}</span></td>
              <td><span class="${chipClass}">${p.state.toUpperCase()}</span></td>
              <td>
                <select class="form-control form-control-sm" onchange="Processes.changeState(${p.pid}, this.value)">
                  <option value="New" ${p.state === 'New' ? 'selected' : ''}>New</option>
                  <option value="Ready" ${p.state === 'Ready' ? 'selected' : ''}>Ready</option>
                  <option value="Running" ${p.state === 'Running' ? 'selected' : ''}>Running</option>
                  <option value="Waiting" ${p.state === 'Waiting' ? 'selected' : ''}>Waiting</option>
                  <option value="Terminated" ${p.state === 'Terminated' ? 'selected' : ''}>Terminated</option>
                </select>
              </td>
              <td>
                <div style="display:flex; gap:0.4rem;">
                  <button class="btn btn-outline btn-sm" onclick="Processes.terminate(${p.pid})" title="Terminate process">
                    Term
                  </button>
                  <button class="btn btn-danger btn-sm" onclick="Processes.delete(${p.pid})" title="Remove process">
                    ✕
                  </button>
                </div>
              </td>
            </tr>
          `;
        }).join('');
      }
    }

    // 2. PCB Table
    const tbodyPCB = document.getElementById('tbody-pcbs');
    if (tbodyPCB) {
      if (this.currentPCBs.length === 0) {
        tbodyPCB.innerHTML = `<tr><td colspan="5" style="text-align:center; color:var(--text-dim); padding:2rem;">No PCBs allocated in kernel.</td></tr>`;
      } else {
        tbodyPCB.innerHTML = this.currentPCBs.map(pcb => {
          const pcHex = '0x' + pcb.programCounter.toString(16).padStart(4, '0').toUpperCase();
          const stateLower = pcb.state.toLowerCase();
          return `
            <tr>
              <td><strong>P${pcb.pid}</strong></td>
              <td><span class="chip chip-${stateLower}">${pcb.state.toUpperCase()}</span></td>
              <td><code>Level ${pcb.priority}</code></td>
              <td><code>${pcHex}</code></td>
              <td>
                <button class="btn btn-outline btn-sm" onclick="Unified.inspectProcess(${pcb.pid})">
                  Inspect PCB
                </button>
              </td>
            </tr>
          `;
        }).join('');
      }
    }
  },

  async handleCreate(e) {
    e.preventDefault();
    const name = document.getElementById('input-proc-name').value.trim();
    const pidVal = document.getElementById('input-proc-pid').value;
    const arrival = parseInt(document.getElementById('input-proc-arrival').value, 10) || 0;
    const burst = parseInt(document.getElementById('input-proc-burst').value, 10) || 1;
    const priority = parseInt(document.getElementById('input-proc-priority').value, 10) || 1;

    const payload = {
      name: name || undefined,
      pid: pidVal ? parseInt(pidVal, 10) : undefined,
      arrivalTime: arrival,
      burstTime: burst,
      priority: priority
    };

    try {
      await API.createProcess(payload);
      App.showToast(`Process ${name || 'P'} created successfully!`, 'success');
      document.getElementById('form-create-process').reset();
      document.getElementById('input-proc-arrival').value = '0';
      document.getElementById('input-proc-burst').value = '4';
      document.getElementById('input-proc-priority').value = '2';
      await this.refresh();
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Failed to create process', 'error');
    }
  },

  async changeState(pid, newState) {
    try {
      await API.changeState(pid, newState);
      App.showToast(`P${pid} state changed to ${newState}`, 'info');
      await this.refresh();
      await Dashboard.refresh();
      await CPU.refresh();
    } catch (err) {
      App.showToast('Failed to update state', 'error');
    }
  },

  async terminate(pid) {
    try {
      await API.changeState(pid, 'Terminated');
      App.showToast(`P${pid} terminated`, 'info');
      await this.refresh();
      await Dashboard.refresh();
      await CPU.refresh();
    } catch (err) {
      App.showToast('Failed to terminate process', 'error');
    }
  },

  async delete(pid) {
    try {
      await API.deleteProcess(pid);
      App.showToast(`P${pid} removed from kernel table`, 'info');
      await this.refresh();
      await Dashboard.refresh();
      await CPU.refresh();
    } catch (err) {
      App.showToast('Failed to delete process', 'error');
    }
  },

  async loadPreset(type) {
    try {
      if (type === 'default') {
        await API.resetProcesses();
        App.showToast('Loaded Default Set: P1, P2, P3', 'success');
      } else if (type === 'mixed') {
        await API.resetProcesses();
        await API.createProcess({ pid: 4, name: 'P4', arrivalTime: 4, burstTime: 6, priority: 2 });
        await API.createProcess({ pid: 5, name: 'P5', arrivalTime: 6, burstTime: 1, priority: 4 });
        App.showToast('Loaded Mixed Arrivals Set: P1 through P5', 'success');
      }
      await this.refresh();
      await Dashboard.refresh();
      await CPU.refresh();
    } catch (err) {
      App.showToast('Failed to load preset', 'error');
    }
  },

  updateOtherModuleDropdowns() {
    // 1. CPU dispatch dropdown
    const selectCpu = document.getElementById('select-cpu-dispatch');
    if (selectCpu) {
      const readyProcs = this.currentProcesses.filter(p => p.state === 'Ready' || p.state === 'New');
      if (readyProcs.length === 0) {
        selectCpu.innerHTML = `<option value="">No Ready Processes</option>`;
      } else {
        selectCpu.innerHTML = readyProcs.map(p => `
          <option value="${p.pid}">${p.name} (PID: ${p.pid}, Burst: ${p.burstTime}, Priority: ${p.priority})</option>
        `).join('');
      }
    }

    // 2. Memory PID dropdown
    const selectMem = document.getElementById('select-mem-pid');
    if (selectMem) {
      if (this.currentProcesses.length === 0) {
        selectMem.innerHTML = `<option value="1">P1 (Default)</option>`;
      } else {
        selectMem.innerHTML = this.currentProcesses.map(p => `
          <option value="${p.pid}">${p.name} (PID: ${p.pid})</option>
        `).join('');
      }
    }

    // 3. Unified Inspector Chips
    const unifiedChips = document.getElementById('unified-proc-chips');
    if (unifiedChips) {
      if (this.currentProcesses.length === 0) {
        unifiedChips.innerHTML = `<span style="color:var(--text-dim)">No active processes.</span>`;
      } else {
        unifiedChips.innerHTML = this.currentProcesses.map(p => `
          <button class="proc-chip-btn" id="chip-proc-${p.pid}" onclick="Unified.inspectProcess(${p.pid})">
            ${p.name} (PID: ${p.pid})
          </button>
        `).join('');
      }
    }
  }
};

