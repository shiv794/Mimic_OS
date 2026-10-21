/**
 * Mimic OS - API Client Bridge
 * Communicates with C++ HTTP Server / REST API
 */
const API = {
  baseUrl: window.location.origin.includes('http') ? window.location.origin : 'http://localhost:8080',
  isOnline: false,

  async request(endpoint, options = {}) {
    const url = `${this.baseUrl}${endpoint}`;
    try {
      const res = await fetch(url, {
        headers: {
          'Content-Type': 'application/json',
          ...(options.headers || {})
        },
        ...options
      });

      if (!res.ok) {
        throw new Error(`HTTP ${res.status}: ${res.statusText}`);
      }

      this.isOnline = true;
      this.updateConnectionBadge(true);
      return await res.json();
    } catch (err) {
      this.isOnline = false;
      this.updateConnectionBadge(false);
      console.warn(`[API] Request to ${endpoint} failed:`, err.message);
      throw err;
    }
  },

  updateConnectionBadge(online) {
    const el = document.getElementById('val-backend');
    const tb = document.getElementById('tb-backend');
    if (!el || !tb) return;
    const dot = tb.querySelector('.dot');

    if (online) {
      el.textContent = 'ONLINE (Port 8080)';
      el.style.color = 'var(--state-running)';
      if (dot) {
        dot.className = 'dot online';
      }
    } else {
      el.textContent = 'OFFLINE (Start Server)';
      el.style.color = 'var(--accent-danger)';
      if (dot) {
        dot.className = 'dot';
        dot.style.backgroundColor = 'var(--accent-danger)';
      }
    }
  },

  // Telemetry
  async getStatus() {
    return this.request('/api/status');
  },

  // Processes
  async getProcesses() {
    return this.request('/api/processes');
  },

  async createProcess(proc) {
    return this.request('/api/processes/create', {
      method: 'POST',
      body: JSON.stringify(proc)
    });
  },

  async deleteProcess(pid) {
    return this.request('/api/processes/delete', {
      method: 'POST',
      body: JSON.stringify({ pid })
    });
  },

  async changeState(pid, state) {
    return this.request('/api/processes/state', {
      method: 'POST',
      body: JSON.stringify({ pid, state })
    });
  },

  async resetProcesses() {
    return this.request('/api/processes/reset', {
      method: 'POST',
      body: JSON.stringify({})
    });
  },

  // CPU
  async getCPU() {
    return this.request('/api/cpu');
  },

  async executeCPU(pid) {
    return this.request('/api/cpu/execute', {
      method: 'POST',
      body: JSON.stringify({ pid })
    });
  },

  async stopCPU() {
    return this.request('/api/cpu/stop', {
      method: 'POST',
      body: JSON.stringify({})
    });
  },

  // Memory
  async getMemory() {
    return this.request('/api/memory');
  },

  async allocateMemory(algorithm, pid, size) {
    return this.request('/api/memory/allocate', {
      method: 'POST',
      body: JSON.stringify({ algorithm, pid, size })
    });
  },

  async freeMemory(pid) {
    return this.request('/api/memory/free', {
      method: 'POST',
      body: JSON.stringify({ pid })
    });
  },

  async resetMemory() {
    return this.request('/api/memory/reset', {
      method: 'POST',
      body: JSON.stringify({})
    });
  },

  // Scheduling Simulation
  async runSchedule(algorithm, quantum = 2) {
    return this.request('/api/schedule/run', {
      method: 'POST',
      body: JSON.stringify({ algorithm, quantum })
    });
  }
};

