/**
 * Mimic OS - Deadlock Detection Controller
 * Banker's / Resource-Allocation-Graph Deadlock Simulator
 */
const Deadlock = {
  data: null,

  init() {
    const btnDetect = document.getElementById('btn-deadlock-detect');
    if (btnDetect) {
      btnDetect.addEventListener('click', () => this.runDetection());
    }

    const btnSafe = document.getElementById('btn-load-safe-scenario');
    if (btnSafe) {
      btnSafe.addEventListener('click', () => this.loadScenario('safe'));
    }

    const btnDeadlock = document.getElementById('btn-load-deadlock-scenario');
    if (btnDeadlock) {
      btnDeadlock.addEventListener('click', () => this.loadScenario('deadlock'));
    }
  },

  async refresh() {
    try {
      this.data = await API.getDeadlock();
      this.render();
    } catch (err) {
      console.warn('[Deadlock] Refresh failed:', err.message);
    }
  },

  async runDetection() {
    try {
      this.data = await API.detectDeadlock();
      this.render();
      if (this.data.isDeadlocked) {
        App.showToast(`Deadlock detected! Processes: ${this.data.deadlockedProcesses.map(p => 'P' + p).join(', ')}`, 'error');
      } else {
        App.showToast(`No deadlock detected! System is in a safe state.`, 'success');
      }
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Detection execution failed', 'error');
    }
  },

  async loadScenario(scenario) {
    try {
      this.data = await API.loadDeadlockScenario(scenario);
      this.render();
      App.showToast(`Loaded ${scenario === 'safe' ? 'Safe' : 'Deadlock'} test scenario`, 'info');
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Failed to load scenario', 'error');
    }
  },

  async updateCell(type, process, resource, value) {
    try {
      const val = parseInt(value, 10);
      if (isNaN(val) || val < 0) return;

      this.data = await API.updateDeadlock({
        type: type,
        process: process,
        resource: resource,
        value: val
      });
      this.render();
      App.showToast(`Updated ${type} [P${process}, R${resource}] = ${val}`, 'info');
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Update failed', 'error');
    }
  },

  async updateAvailable(resource, value) {
    try {
      const val = parseInt(value, 10);
      if (isNaN(val) || val < 0) return;

      this.data = await API.updateDeadlock({
        type: 'available',
        resource: resource,
        value: val
      });
      this.render();
      App.showToast(`Updated Available [R${resource}] = ${val}`, 'info');
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Update failed', 'error');
    }
  },

  render() {
    if (!this.data) return;

    const {
      processCount, resourceCount,
      allocation, request, available,
      isDeadlocked,
      deadlockedProcesses,  // row indices for matrix highlighting
      deadlockedPids,       // actual PIDs for display
      safeSequenceNames,    // actual names for safe sequence display
      processIds,           // actual PIDs per row
      processNames          // actual names per row
    } = this.data;

    // Helper: get display name for row i
    const rowName = (i) => (processNames && processNames[i]) ? processNames[i] : `P${i}`;
    const rowPid  = (i) => (processIds  && processIds[i]  != null) ? processIds[i]  : i;

    // 1. Render Status Alert Banner
    const banner = document.getElementById('deadlock-status-banner');
    const badge = document.getElementById('deadlock-status-badge');
    const title = document.getElementById('deadlock-status-title');
    const desc = document.getElementById('deadlock-status-desc');

    if (banner && badge && title && desc) {
      if (isDeadlocked) {
        banner.className = 'card deadlock-alert-banner banner-danger';
        badge.className = 'badge badge-danger';
        badge.textContent = 'DEADLOCK DETECTED';
        title.textContent = 'Deadlock Detected — Circular Wait Exists';
        title.style.color = 'var(--accent-danger)';

        // Use PID-based names if available, fall back to row-index names
        const deadNames = (deadlockedPids && deadlockedPids.length > 0)
          ? deadlockedPids.map(pid => `<strong>P${pid}</strong>`).join(', ')
          : (deadlockedProcesses || []).map(i => `<strong>${rowName(i)}</strong>`).join(', ');
        desc.innerHTML = `Processes ${deadNames} are blocked waiting for resources held by each other. None of these processes can proceed to completion.`;
      } else {
        banner.className = 'card deadlock-alert-banner banner-success';
        badge.className = 'badge badge-success';
        badge.textContent = 'SAFE STATE';
        title.textContent = 'No Deadlock Detected — System Can Complete Safely';
        title.style.color = 'var(--state-running)';

        const seqStr = (safeSequenceNames && safeSequenceNames.length > 0)
          ? safeSequenceNames.join(' ➔ ')
          : 'All Processes Completed';
        desc.innerHTML = `All processes can proceed to completion without deadlock.<br><strong>Executable Sequence:</strong> <code class="safe-seq-code">${seqStr}</code>`;
      }
    }

    // 2. Render Process & Resource Visual Nodes
    const procNodesContainer = document.getElementById('deadlock-proc-nodes');
    if (procNodesContainer) {
      let html = '';
      for (let i = 0; i < processCount; i++) {
        const isDead = deadlockedProcesses && deadlockedProcesses.includes(i);
        const nodeClass = isDead ? 'node-deadlocked' : 'node-safe';
        const label = isDead ? 'DEADLOCKED' : 'RUNNABLE';
        const name = rowName(i);

        html += `
          <div class="dl-node ${nodeClass}">
            <div class="dl-node-header">
              <span class="dl-node-id">${name}</span>
              <span class="dl-node-status">${label}</span>
            </div>
            <div class="dl-node-body">
              <div>Alloc: <code>[${(allocation[i] || []).join(',')}]</code></div>
              <div>Req: <code>[${(request[i] || []).join(',')}]</code></div>
            </div>
          </div>
        `;
      }
      procNodesContainer.innerHTML = html;
    }

    // 3. Render Available Resources Bar
    const availContainer = document.getElementById('deadlock-available-bar');
    if (availContainer) {
      availContainer.innerHTML = available.map((val, idx) => `
        <div class="avail-res-box">
          <span class="res-label">Resource R${idx}</span>
          <div class="avail-input-wrap">
            <input type="number" min="0" max="99" value="${val}" class="avail-input" 
                   onchange="Deadlock.updateAvailable(${idx}, this.value)" title="Click to edit available units">
          </div>
        </div>
      `).join('');
    }

    // 4. Render Allocation Matrix Table
    const allocThead = document.getElementById('deadlock-alloc-thead');
    const allocTbody = document.getElementById('deadlock-alloc-tbody');
    if (allocThead && allocTbody) {
      let ths = '<th>Process</th>';
      for (let j = 0; j < resourceCount; j++) ths += `<th>R${j}</th>`;
      allocThead.innerHTML = `<tr>${ths}</tr>`;

      allocTbody.innerHTML = allocation.map((row, pIdx) => {
        const isDead = deadlockedProcesses && deadlockedProcesses.includes(pIdx);
        const name = rowName(pIdx);
        let tds = `<td><strong class="${isDead ? 'text-danger' : ''}">${name}</strong></td>`;
        for (let rIdx = 0; rIdx < resourceCount; rIdx++) {
          tds += `
            <td>
              <input type="number" min="0" max="99" class="matrix-cell-input" value="${row[rIdx] || 0}"
                     onchange="Deadlock.updateCell('allocation', ${pIdx}, ${rIdx}, this.value)">
            </td>
          `;
        }
        return `<tr>${tds}</tr>`;
      }).join('');
    }

    // 5. Render Request Matrix Table
    const reqThead = document.getElementById('deadlock-req-thead');
    const reqTbody = document.getElementById('deadlock-req-tbody');
    if (reqThead && reqTbody) {
      let ths = '<th>Process</th>';
      for (let j = 0; j < resourceCount; j++) ths += `<th>R${j}</th>`;
      reqThead.innerHTML = `<tr>${ths}</tr>`;

      reqTbody.innerHTML = request.map((row, pIdx) => {
        const isDead = deadlockedProcesses && deadlockedProcesses.includes(pIdx);
        const name = rowName(pIdx);
        let tds = `<td><strong class="${isDead ? 'text-danger' : ''}">${name}</strong></td>`;
        for (let rIdx = 0; rIdx < resourceCount; rIdx++) {
          tds += `
            <td>
              <input type="number" min="0" max="99" class="matrix-cell-input" value="${row[rIdx] || 0}"
                     onchange="Deadlock.updateCell('request', ${pIdx}, ${rIdx}, this.value)">
            </td>
          `;
        }
        return `<tr>${tds}</tr>`;
      }).join('');
    }
  }
};

