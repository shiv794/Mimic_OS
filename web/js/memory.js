/**
 * Mimic OS - Memory Management Controller
 * Contiguous dynamic partitioning with First Fit, Best Fit, Worst Fit & Hole Coalescing
 */
const Memory = {
  currentMemoryData: null,

  init() {
    const form = document.getElementById('form-allocate-memory');
    if (form) {
      form.addEventListener('submit', (e) => this.handleAllocate(e));
    }

    const btnReset = document.getElementById('btn-mem-reset');
    if (btnReset) {
      btnReset.addEventListener('click', () => this.handleReset());
    }
  },

  async refresh() {
    try {
      const data = await API.getMemory();
      this.currentMemoryData = data;
      this.render();
    } catch (err) {
      console.warn('[Memory] Refresh failed:', err.message);
    }
  },

  render() {
    if (!this.currentMemoryData) return;
    const { totalMemory, usedMemory, freeMemory, blocks } = this.currentMemoryData;

    // Update summary pills
    const elUsed = document.getElementById('mem-view-used');
    const elFree = document.getElementById('mem-view-free');
    if (elUsed) elUsed.textContent = `${usedMemory} MB`;
    if (elFree) elFree.textContent = `${freeMemory} MB`;

    // 1. Proportional Memory Map Bar
    const bar = document.getElementById('memory-map-bar');
    if (bar) {
      bar.innerHTML = blocks.map((b, idx) => {
        const widthPct = Math.max(3, (b.size / totalMemory) * 100);
        const isAlloc = b.allocated;
        const color = isAlloc ? Scheduler.getProcessColor(b.pid) : 'transparent';
        const cellClass = `memory-block-cell ${isAlloc ? 'allocated' : 'free'}`;

        return `
          <div class="${cellClass}" id="mem-block-${idx}" style="width: ${widthPct}%; ${isAlloc ? `background-color: ${color};` : ''}" 
               title="Block #${idx}: [0x${b.startAddress.toString(16).padStart(4, '0').toUpperCase()}] ${b.size} MB ${isAlloc ? `(P${b.pid})` : '(Free Hole)'}">
            <span class="cell-title">${isAlloc ? `P${b.pid}` : 'FREE'}</span>
            <span class="cell-size">${b.size} MB</span>
          </div>
        `;
      }).join('');
    }

    // 2. Memory Blocks Table
    const tbody = document.getElementById('tbody-memory-blocks');
    if (tbody) {
      tbody.innerHTML = blocks.map((b, idx) => {
        const startHex = '0x' + b.startAddress.toString(16).padStart(4, '0').toUpperCase();
        const endHex = '0x' + (b.startAddress + b.size).toString(16).padStart(4, '0').toUpperCase();
        const isAlloc = b.allocated;

        return `
          <tr>
            <td><strong>#${idx}</strong></td>
            <td><code>${startHex} (${b.startAddress} MB)</code></td>
            <td><code>${endHex} (${b.startAddress + b.size} MB)</code></td>
            <td><strong>${b.size} MB</strong></td>
            <td>
              ${isAlloc ? `<span class="badge" style="background:${Scheduler.getProcessColor(b.pid)}; color:#fff;">P${b.pid}</span>` : '<span style="color:var(--text-dim)">--</span>'}
            </td>
            <td>
              <span class="chip ${isAlloc ? 'chip-running' : 'chip-ready'}">
                ${isAlloc ? 'ALLOCATED' : 'FREE HOLE'}
              </span>
            </td>
            <td>
              ${isAlloc ? `
                <button class="btn btn-danger btn-sm" onclick="Memory.free(${b.pid})">
                  Free Memory
                </button>
              ` : `
                <span style="font-size:0.75rem; color:var(--text-dim);">Available</span>
              `}
            </td>
          </tr>
        `;
      }).join('');
    }
  },

  async handleAllocate(e) {
    e.preventDefault();
    const algo = document.getElementById('select-mem-strategy').value;
    const pid = parseInt(document.getElementById('select-mem-pid').value, 10) || 1;
    const size = parseInt(document.getElementById('input-mem-size').value, 10) || 100;

    try {
      const res = await API.allocateMemory(algo, pid, size);
      
      const explainer = document.getElementById('mem-explanation-text');
      const explainerTag = document.getElementById('mem-explainer-tag');

      if (res.success) {
        if (explainerTag) {
          explainerTag.textContent = algo.toUpperCase();
          explainerTag.className = 'card-tag badge-accent';
        }
        if (explainer) {
          explainer.innerHTML = `<strong>Allocation Succeeded:</strong> ${res.explanation}`;
        }
        App.showToast(`Allocated ${size} MB to P${pid} via ${algo}`, 'success');
      } else {
        if (explainerTag) {
          explainerTag.textContent = 'ALLOCATION FAILED';
          explainerTag.className = 'card-tag badge-danger';
        }
        if (explainer) {
          explainer.innerHTML = `<span style="color:var(--accent-danger)"><strong>Out of Memory:</strong> ${res.explanation}</span>`;
        }
        App.showToast('Not enough contiguous memory', 'error');
      }

      await this.refresh();
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Allocation failed', 'error');
    }
  },

  async free(pid) {
    try {
      const res = await API.freeMemory(pid);
      const explainer = document.getElementById('mem-explanation-text');
      const explainerTag = document.getElementById('mem-explainer-tag');

      if (explainerTag) explainerTag.textContent = 'COALESCING COMPLETED';
      if (explainer) {
        explainer.innerHTML = `<strong>Memory Deallocated:</strong> Released blocks for P${pid}. The C++ Memory Manager inspected adjacent free memory holes and coalesced them into a unified larger hole.`;
      }

      App.showToast(`Freed memory for P${pid}`, 'info');
      await this.refresh();
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Failed to free memory', 'error');
    }
  },

  async handleReset() {
    try {
      await API.resetMemory();
      const explainer = document.getElementById('mem-explanation-text');
      if (explainer) {
        explainer.textContent = 'Physical RAM reset: All allocations cleared. Contiguous single hole of 1024 MB restored at address 0x0000.';
      }
      App.showToast('Physical memory reset to 1024 MB', 'info');
      await this.refresh();
      await Dashboard.refresh();
    } catch (err) {
      App.showToast('Failed to reset memory', 'error');
    }
  }
};

