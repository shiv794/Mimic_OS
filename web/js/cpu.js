/**
 * Mimic OS - CPU Execution Core Controller
 */
const CPU = {
  isBusy: false,
  currentProcess: null,

  init() {
    const btnDispatch = document.getElementById('btn-cpu-dispatch');
    if (btnDispatch) {
      btnDispatch.addEventListener('click', () => this.handleDispatch());
    }

    const btnStop = document.getElementById('btn-cpu-stop');
    if (btnStop) {
      btnStop.addEventListener('click', () => this.handleStop());
    }
  },

  async refresh() {
    try {
      const data = await API.getCPU();
      this.render(data);
    } catch (err) {
      console.warn('[CPU] Refresh failed:', err.message);
    }
  },

  render(data) {
    this.isBusy = data.busy;
    this.currentProcess = data.currentProcess;

    const socket = document.getElementById('cpu-socket-element');
    const dieStatus = document.getElementById('cpu-die-status');
    const avatar = document.getElementById('cpu-proc-avatar');
    const procName = document.getElementById('cpu-proc-name');
    const procPid = document.getElementById('cpu-proc-pid');
    const burstBar = document.getElementById('cpu-burst-bar');
    const burstVal = document.getElementById('cpu-burst-val');

    const regPid = document.getElementById('reg-pid');
    const regPc = document.getElementById('reg-pc');
    const regStatus = document.getElementById('reg-status');
    const regLast = document.getElementById('reg-last-proc');

    if (this.isBusy && this.currentProcess) {
      const p = this.currentProcess;
      if (socket) socket.classList.add('running');
      if (dieStatus) {
        dieStatus.className = 'cpu-status-pill running';
        dieStatus.textContent = 'RUNNING';
      }
      if (avatar) {
        avatar.className = 'process-avatar running';
        avatar.textContent = `P${p.pid}`;
      }
      if (procName) procName.textContent = p.name;
      if (procPid) procPid.textContent = `PID: ${p.pid} • Priority: ${p.priority}`;
      if (burstBar) burstBar.style.width = '75%';
      if (burstVal) burstVal.textContent = `${p.burstTime} units allocated`;

      if (regPid) regPid.textContent = '0x' + p.pid.toString(16).padStart(4, '0').toUpperCase();
      if (regPc) regPc.textContent = '0x0004';
      if (regStatus) {
        regStatus.textContent = 'RUNNING';
        regStatus.style.color = 'var(--state-running)';
      }
    } else {
      if (socket) socket.classList.remove('running');
      if (dieStatus) {
        dieStatus.className = 'cpu-status-pill';
        dieStatus.textContent = 'IDLE';
      }
      if (avatar) {
        avatar.className = 'process-avatar';
        avatar.textContent = '--';
      }
      if (procName) procName.textContent = 'CPU IDLE';
      if (procPid) procPid.textContent = 'No active dispatch';
      if (burstBar) burstBar.style.width = '0%';
      if (burstVal) burstVal.textContent = '0 / 0 units';

      if (regPid) regPid.textContent = '0x0000';
      if (regPc) regPc.textContent = '0x0000';
      if (regStatus) {
        regStatus.textContent = 'IDLE';
        regStatus.style.color = 'var(--accent-cyan)';
      }
    }

    if (regLast) {
      regLast.textContent = data.lastProcessName || 'None';
    }
  },

  async handleDispatch() {
    const select = document.getElementById('select-cpu-dispatch');
    if (!select || !select.value) {
      App.showToast('No process selected to dispatch', 'error');
      return;
    }

    const pid = parseInt(select.value, 10);
    try {
      await API.executeCPU(pid);
      this.logEvent(`Dispatched process P${pid} to CPU Core`);
      App.showToast(`P${pid} dispatched to CPU`, 'success');
      await this.refresh();
      await Processes.refresh();
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Dispatch failed', 'error');
    }
  },

  async handleStop() {
    try {
      await API.stopCPU();
      this.logEvent('CPU Core halted by operator');
      App.showToast('CPU halted', 'info');
      await this.refresh();
      await Processes.refresh();
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Stop failed', 'error');
    }
  },

  logEvent(msg) {
    const feed = document.getElementById('cpu-activity-log');
    if (!feed) return;
    const now = new Date();
    const timeStr = `[${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}]`;
    const entry = document.createElement('div');
    entry.className = 'log-entry';
    entry.innerHTML = `<span class="log-time">${timeStr}</span><span class="log-msg">${msg}</span>`;
    feed.insertBefore(entry, feed.firstChild);
  }
};

