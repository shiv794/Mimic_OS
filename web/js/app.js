/**
 * Mimic OS - Main Application Controller
 * Handles Navigation, Topbar Clock, Telemetry Polling, Toasts, and Hotkeys
 */
const App = {
  currentView: 'dashboard',
  pollingTimer: null,

  init() {
    this.setupNavigation();
    this.setupClock();
    this.setupQuickReset();
    this.setupKeyboardShortcuts();

    // Initialize all module controllers
    Processes.init();
    CPU.init();
    Scheduler.init();
    Memory.init();
    Unified.init();

    // Initial data refresh
    this.refreshAll();

    // Periodic live status polling (every 2.5s)
    this.pollingTimer = setInterval(() => {
      this.pollBackgroundTelemetry();
    }, 2500);
  },

  setupNavigation() {
    const navItems = document.querySelectorAll('.nav-item');
    navItems.forEach(item => {
      item.addEventListener('click', () => {
        const view = item.dataset.view;
        if (view) this.navigateTo(view);
      });
    });
  },

  navigateTo(viewId) {
    this.currentView = viewId;

    // Update Sidebar Navigation
    document.querySelectorAll('.nav-item').forEach(item => {
      if (item.dataset.view === viewId) {
        item.classList.add('active');
      } else {
        item.classList.remove('active');
      }
    });

    // Switch View Panel
    document.querySelectorAll('.view-panel').forEach(panel => {
      panel.classList.remove('active');
    });

    const targetPanel = document.getElementById(`view-${viewId}`);
    if (targetPanel) {
      targetPanel.classList.add('active');
    }

    // Module-specific refresh on activation
    if (viewId === 'processes') Processes.refresh();
    if (viewId === 'cpu') CPU.refresh();
    if (viewId === 'memory') Memory.refresh();
    if (viewId === 'dashboard') Dashboard.refresh();
  },

  async refreshAll() {
    await Promise.allSettled([
      Dashboard.refresh(),
      Processes.refresh(),
      CPU.refresh(),
      Memory.refresh()
    ]);
  },

  async pollBackgroundTelemetry() {
    // Light telemetry ping to keep topbar and dashboard updated
    try {
      await Dashboard.refresh();
      if (this.currentView === 'cpu') await CPU.refresh();
      if (this.currentView === 'processes') await Processes.refresh();
      if (this.currentView === 'memory') await Memory.refresh();
    } catch (e) {
      // offline handled gracefully
    }
  },

  setupClock() {
    const clockEl = document.getElementById('clock-display');
    const update = () => {
      const now = new Date();
      if (clockEl) {
        clockEl.textContent = now.toTimeString().split(' ')[0];
      }
    };
    update();
    setInterval(update, 1000);
  },

  setupQuickReset() {
    const btn = document.getElementById('btn-quick-reset');
    if (btn) {
      btn.addEventListener('click', async () => {
        try {
          await API.resetProcesses();
          await API.resetMemory();
          this.showToast('Reset kernel state to default configuration', 'success');
          await this.refreshAll();
          Scheduler.resetSimulation();
        } catch (err) {
          this.showToast('Reset failed', 'error');
        }
      });
    }
  },

  setupKeyboardShortcuts() {
    window.addEventListener('keydown', (e) => {
      // Do not trigger if user is typing in an input
      if (['INPUT', 'SELECT', 'TEXTAREA'].includes(document.activeElement.tagName)) {
        return;
      }

      if (e.key === '1') this.navigateTo('dashboard');
      if (e.key === '2') this.navigateTo('processes');
      if (e.key === '3') this.navigateTo('cpu');
      if (e.key === '4') this.navigateTo('scheduler');
      if (e.key === '5') this.navigateTo('memory');
      if (e.key === '6') this.navigateTo('unified');

      if (e.code === 'Space' && this.currentView === 'scheduler') {
        e.preventDefault();
        Scheduler.togglePlay();
      }

      if ((e.key === 's' || e.key === 'S') && this.currentView === 'scheduler') {
        e.preventDefault();
        Scheduler.stepForward();
      }
    });
  },

  showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;

    let icon = 'ℹ️';
    if (type === 'success') icon = '✓';
    if (type === 'error') icon = '⚠️';

    toast.innerHTML = `<span>${icon}</span><span>${message}</span>`;
    container.appendChild(toast);

    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateY(10px)';
      toast.style.transition = 'all 0.2s';
      setTimeout(() => toast.remove(), 250);
    }, 3000);
  }
};

// Bootstrap on DOM ready
document.addEventListener('DOMContentLoaded', () => {
  App.init();
});

