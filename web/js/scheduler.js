/**
 * Mimic OS - CPU Scheduling Simulator & Visualizer
 * Dynamic Gantt Chart, Ready Queue Conveyor, Round Robin Circular Queue & Step-by-Step Playback
 */
const Scheduler = {
  simulationData: null,
  currentStepIndex: 0,
  isPlaying: false,
  playbackTimer: null,
  playbackSpeed: 1000, // ms per step

  // Distinct process color map
  processColors: {
    1: '#10b981', // Emerald
    2: '#3b82f6', // Blue
    3: '#f59e0b', // Amber
    4: '#8b5cf6', // Purple
    5: '#ec4899', // Pink
    6: '#06b6d4', // Cyan
    7: '#14b8a6', // Teal
    8: '#f97316'  // Orange
  },

  getProcessColor(pid) {
    if (pid <= 0) return '#1e293b';
    return this.processColors[pid] || '#3b82f6';
  },

  init() {
    const selectAlgo = document.getElementById('select-algorithm');
    if (selectAlgo) {
      selectAlgo.addEventListener('change', () => this.handleAlgorithmChange());
    }

    const btnPlay = document.getElementById('btn-sim-play');
    if (btnPlay) {
      btnPlay.addEventListener('click', () => this.togglePlay());
    }

    const btnStep = document.getElementById('btn-sim-step');
    if (btnStep) {
      btnStep.addEventListener('click', () => this.stepForward());
    }

    const btnPrev = document.getElementById('btn-sim-prev');
    if (btnPrev) {
      btnPrev.addEventListener('click', () => this.stepBackward());
    }

    const btnReset = document.getElementById('btn-sched-reset');
    if (btnReset) {
      btnReset.addEventListener('click', () => this.resetSimulation());
    }

    const speedSlider = document.getElementById('slider-speed');
    if (speedSlider) {
      speedSlider.addEventListener('input', (e) => {
        const val = parseInt(e.target.value, 10);
        const speeds = { 1: 1500, 2: 1000, 3: 500, 4: 250 };
        const labels = { 1: '0.5x', 2: '1x', 3: '2x', 4: '4x' };
        this.playbackSpeed = speeds[val] || 1000;
        document.getElementById('label-speed').textContent = labels[val] || '1x';
      });
    }

    this.handleAlgorithmChange();
  },

  handleAlgorithmChange() {
    const selectAlgo = document.getElementById('select-algorithm');
    const quantumGroup = document.getElementById('quantum-control-group');
    const queueTag = document.getElementById('queue-mode-tag');

    if (selectAlgo.value === 'RoundRobin') {
      if (quantumGroup) quantumGroup.style.display = 'flex';
      if (queueTag) queueTag.textContent = 'CIRCULAR READY QUEUE (TIME SLICED)';
    } else {
      if (quantumGroup) quantumGroup.style.display = 'none';
      if (queueTag) queueTag.textContent = 'PRIORITY / FIFO READY QUEUE';
    }

    this.resetSimulation();
  },

  async runSimulation() {
    const algo = document.getElementById('select-algorithm').value;
    const quantum = parseInt(document.getElementById('input-quantum').value, 10) || 2;

    try {
      this.simulationData = await API.runSchedule(algo, quantum);
      this.currentStepIndex = 0;
      this.renderFullGantt();
      this.renderMetricsTable();
      this.applyStep(0);
      App.showToast(`Simulated ${algo} with C++ engine`, 'success');
    } catch (err) {
      App.showToast('Scheduling simulation failed', 'error');
    }
  },

  renderFullGantt() {
    const container = document.getElementById('gantt-chart-container');
    const ruler = document.getElementById('gantt-ruler');
    if (!container || !this.simulationData) return;

    container.innerHTML = '';
    ruler.innerHTML = '';

    const totalTime = Math.max(1, this.simulationData.totalTime);
    const gantt = this.simulationData.gantt || [];

    // Render Gantt Blocks
    gantt.forEach((item, idx) => {
      const duration = item.endTime - item.startTime;
      const widthPct = (duration / totalTime) * 100;
      const block = document.createElement('div');
      block.className = `gantt-block ${item.pid === -1 ? 'idle' : ''}`;
      block.id = `gantt-block-${idx}`;
      block.style.width = `${widthPct}%`;

      if (item.pid !== -1) {
        block.style.backgroundColor = this.getProcessColor(item.pid);
        block.textContent = item.name;
        block.title = `${item.name}: [${item.startTime} -> ${item.endTime}] (${duration} units)`;
      } else {
        block.textContent = 'IDLE';
        block.title = `IDLE: [${item.startTime} -> ${item.endTime}]`;
      }

      container.appendChild(block);
    });

    // Render Timeline Ticks
    const timePoints = new Set([0]);
    gantt.forEach(item => {
      timePoints.add(item.startTime);
      timePoints.add(item.endTime);
    });

    Array.from(timePoints).sort((a, b) => a - b).forEach(t => {
      const leftPct = (t / totalTime) * 100;
      const tick = document.createElement('div');
      tick.className = 'ruler-tick';
      tick.style.left = `${leftPct}%`;
      tick.textContent = t;
      ruler.appendChild(tick);
    });
  },

  renderMetricsTable() {
    const tbody = document.getElementById('tbody-sched-stats');
    if (!tbody || !this.simulationData) return;

    const stats = this.simulationData.stats || [];
    tbody.innerHTML = stats.map(s => {
      const color = this.getProcessColor(s.pid);
      return `
        <tr>
          <td>
            <span style="display:inline-block; width:10px; height:10px; border-radius:2px; background:${color}; margin-right:6px;"></span>
            <strong>${s.name}</strong> (P${s.pid})
          </td>
          <td><code>${s.arrivalTime}</code></td>
          <td><code>${s.burstTime}</code></td>
          <td><code>${s.priority}</code></td>
          <td><code>${s.startTime >= 0 ? s.startTime : '-'}</code></td>
          <td><code>${s.completionTime}</code></td>
          <td><strong>${s.turnaroundTime}</strong></td>
          <td><strong>${s.waitingTime}</strong></td>
          <td><strong>${s.responseTime}</strong></td>
        </tr>
      `;
    }).join('');

    // Summary cards
    document.getElementById('stat-avg-tat').textContent = this.simulationData.avgTurnaroundTime.toFixed(2);
    document.getElementById('stat-avg-wt').textContent = this.simulationData.avgWaitingTime.toFixed(2);
    document.getElementById('stat-avg-rt').textContent = this.simulationData.avgResponseTime.toFixed(2);
    document.getElementById('stat-cpu-util').textContent = `${this.simulationData.cpuUtilization.toFixed(1)}%`;
    document.getElementById('stat-throughput').textContent = `${this.simulationData.throughput.toFixed(2)} proc/unit`;
  },

  applyStep(stepIndex) {
    if (!this.simulationData || !this.simulationData.steps) return;
    const steps = this.simulationData.steps;
    if (stepIndex < 0 || stepIndex >= steps.length) return;

    const step = steps[stepIndex];
    this.currentStepIndex = stepIndex;

    // 1. Narrator Card
    const badge = document.getElementById('narrator-badge');
    const timeEl = document.getElementById('narrator-time');
    const textEl = document.getElementById('narrator-text');

    if (badge) badge.textContent = `Step ${step.stepNumber} of ${steps.length} • [${step.eventType.toUpperCase()}]`;
    if (timeEl) timeEl.textContent = `Simulation Clock: t = ${step.time}`;
    if (textEl) textEl.textContent = step.description;

    // 2. Ready Queue Visualizer
    const qContainer = document.getElementById('queue-boxes-container');
    if (qContainer) {
      if (!step.readyQueue || step.readyQueue.length === 0) {
        qContainer.innerHTML = `<div class="empty-queue-msg">Ready queue is empty</div>`;
      } else {
        qContainer.innerHTML = step.readyQueue.map(pid => {
          const color = this.getProcessColor(pid);
          return `
            <div class="queue-box" style="border-color:${color};">
              <span>P${pid}</span>
            </div>
          `;
        }).join('');
      }
    }

    // 3. CPU Slot
    const cpuSlot = document.getElementById('queue-cpu-proc');
    const slotCard = document.getElementById('queue-cpu-slot');
    if (cpuSlot) {
      if (step.runningPid > 0) {
        const color = this.getProcessColor(step.runningPid);
        cpuSlot.textContent = `${step.runningName} (P${step.runningPid})`;
        cpuSlot.style.color = '#fff';
        slotCard.style.borderColor = color;
        slotCard.style.boxShadow = `0 0 15px ${color}`;
      } else {
        cpuSlot.textContent = 'IDLE';
        cpuSlot.style.color = 'var(--text-dim)';
        slotCard.style.borderColor = 'var(--border-subtle)';
        slotCard.style.boxShadow = 'none';
      }
    }

    // 4. Preempt Return Arrow for Round Robin
    const returnArrow = document.getElementById('queue-return-arrow');
    if (returnArrow) {
      if (step.eventType === 'preempt') {
        returnArrow.classList.add('visible');
      } else {
        returnArrow.classList.remove('visible');
      }
    }

    // 5. Highlight active block in Gantt Chart
    const gantt = this.simulationData.gantt || [];
    gantt.forEach((item, idx) => {
      const el = document.getElementById(`gantt-block-${idx}`);
      if (!el) return;
      if (step.time >= item.startTime && step.time < item.endTime) {
        el.classList.add('active-executing');
      } else {
        el.classList.remove('active-executing');
      }
    });
  },

  async togglePlay() {
    if (!this.simulationData) {
      await this.runSimulation();
    }

    this.isPlaying = !this.isPlaying;
    const btnPlay = document.getElementById('btn-sim-play');
    const label = document.getElementById('label-sim-play');

    if (this.isPlaying) {
      if (label) label.textContent = 'Pause';
      btnPlay.className = 'btn btn-outline';
      this.playLoop();
    } else {
      if (label) label.textContent = 'Resume';
      btnPlay.className = 'btn btn-success';
      clearTimeout(this.playbackTimer);
    }
  },

  playLoop() {
    if (!this.isPlaying) return;

    if (this.currentStepIndex < this.simulationData.steps.length - 1) {
      this.currentStepIndex++;
      this.applyStep(this.currentStepIndex);
      this.playbackTimer = setTimeout(() => this.playLoop(), this.playbackSpeed);
    } else {
      this.isPlaying = false;
      const label = document.getElementById('label-sim-play');
      const btnPlay = document.getElementById('btn-sim-play');
      if (label) label.textContent = 'Restart';
      if (btnPlay) btnPlay.className = 'btn btn-success';
      App.showToast('Simulation execution completed!', 'info');
    }
  },

  async stepForward() {
    if (!this.simulationData) {
      await this.runSimulation();
      return;
    }

    if (this.currentStepIndex < this.simulationData.steps.length - 1) {
      this.currentStepIndex++;
      this.applyStep(this.currentStepIndex);
    } else {
      App.showToast('Reached end of execution', 'info');
    }
  },

  stepBackward() {
    if (!this.simulationData) return;
    if (this.currentStepIndex > 0) {
      this.currentStepIndex--;
      this.applyStep(this.currentStepIndex);
    }
  },

  resetSimulation() {
    clearTimeout(this.playbackTimer);
    this.isPlaying = false;
    this.simulationData = null;
    this.currentStepIndex = 0;

    const label = document.getElementById('label-sim-play');
    const btnPlay = document.getElementById('btn-sim-play');
    if (label) label.textContent = 'Start Simulation';
    if (btnPlay) btnPlay.className = 'btn btn-success';

    const container = document.getElementById('gantt-chart-container');
    const ruler = document.getElementById('gantt-ruler');
    if (container) container.innerHTML = `<div style="display:flex; align-items:center; justify-content:center; width:100%; color:var(--text-dim); font-size:0.8rem;">Click 'Start Simulation' to generate Gantt chart</div>`;
    if (ruler) ruler.innerHTML = '';

    const tbody = document.getElementById('tbody-sched-stats');
    if (tbody) tbody.innerHTML = `<tr><td colspan="9" style="text-align:center; color:var(--text-dim); padding:1.5rem;">No simulation run yet.</td></tr>`;

    const qContainer = document.getElementById('queue-boxes-container');
    if (qContainer) qContainer.innerHTML = `<div class="empty-queue-msg">Ready queue is empty</div>`;

    const cpuSlot = document.getElementById('queue-cpu-proc');
    if (cpuSlot) cpuSlot.textContent = 'IDLE';

    const narrator = document.getElementById('narrator-text');
    if (narrator) narrator.textContent = 'Select an algorithm and click Start Simulation or Step to observe scheduling.';

    document.getElementById('stat-avg-tat').textContent = '0.00';
    document.getElementById('stat-avg-wt').textContent = '0.00';
    document.getElementById('stat-avg-rt').textContent = '0.00';
    document.getElementById('stat-cpu-util').textContent = '0.0%';
    document.getElementById('stat-throughput').textContent = '0.00';
  }
};

