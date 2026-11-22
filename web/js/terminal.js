/**
 * Mimic OS - Custom Terminal Controller
 * Educational Interactive Shell Bridge
 */
const Terminal = {
  history: [],
  historyIndex: -1,
  isExecuting: false,

  init() {
    const input = document.getElementById('terminal-input');
    if (input) {
      input.addEventListener('keydown', (e) => this.handleKeyDown(e));
    }

    const btnClear = document.getElementById('btn-terminal-clear');
    if (btnClear) {
      btnClear.addEventListener('click', () => this.clearTerminal());
    }

    const btnHelp = document.getElementById('btn-terminal-help');
    if (btnHelp) {
      btnHelp.addEventListener('click', () => this.runCommand('help'));
    }

    // Command suggestion chips
    document.querySelectorAll('.term-pill').forEach(pill => {
      pill.addEventListener('click', () => {
        const cmd = pill.dataset.cmd;
        if (cmd) {
          const inp = document.getElementById('terminal-input');
          if (inp) {
            inp.value = cmd;
            inp.focus();
          }
          this.runCommand(cmd);
        }
      });
    });

    // Initial greeting if empty
    const output = document.getElementById('terminal-output');
    if (output && output.children.length === 0) {
      this.printBanner();
    }
  },

  printBanner() {
    const output = document.getElementById('terminal-output');
    if (!output) return;

    output.innerHTML = `
      <div class="terminal-banner">
======================================================
      MIMIC OS INTERACTIVE SIMULATION SHELL (v2.0)
     BTech Semester 3 PBL • C++ Kernel Integration
======================================================
Type <span class="term-highlight">help</span> for the full list of kernel commands.
Try: <span class="term-code">ps</span>, <span class="term-code">cpu</span>, <span class="term-code">ls</span>, <span class="term-code">memory</span>, <span class="term-code">deadlock</span>, <span class="term-code">fcfs</span>, <span class="term-code">rr 2</span>
      </div>
    `;
  },

  async handleKeyDown(e) {
    const input = document.getElementById('terminal-input');
    if (!input) return;

    if (e.key === 'Enter') {
      e.preventDefault();
      const cmd = input.value.trim();
      if (!cmd) return;

      this.history.push(cmd);
      this.historyIndex = this.history.length;
      input.value = '';

      await this.runCommand(cmd);
    } else if (e.key === 'ArrowUp') {
      e.preventDefault();
      if (this.history.length > 0 && this.historyIndex > 0) {
        this.historyIndex--;
        input.value = this.history[this.historyIndex];
      }
    } else if (e.key === 'ArrowDown') {
      e.preventDefault();
      if (this.historyIndex < this.history.length - 1) {
        this.historyIndex++;
        input.value = this.history[this.historyIndex];
      } else {
        this.historyIndex = this.history.length;
        input.value = '';
      }
    }
  },

  async runCommand(command) {
    if (this.isExecuting) return;
    this.isExecuting = true;

    const output = document.getElementById('terminal-output');
    const input = document.getElementById('terminal-input');

    // Echo command in terminal output
    if (output) {
      const line = document.createElement('div');
      line.className = 'terminal-line-cmd';
      line.innerHTML = `<span class="term-prompt">mimic&gt;</span> <span class="term-echo">${this.escapeHtml(command)}</span>`;
      output.appendChild(line);
      output.scrollTop = output.scrollHeight;
    }

    try {
      const res = await API.executeShell(command);

      if (res.isClear) {
        this.clearTerminal();
      } else {
        if (output) {
          const outBlock = document.createElement('div');
          outBlock.className = 'terminal-line-out';
          outBlock.textContent = res.output || '(Command completed with no output)';
          output.appendChild(outBlock);
          output.scrollTop = output.scrollHeight;
        }

        // Live synchronizations: if command altered processes, files, or memory, refresh respective modules
        if (command.startsWith('create_process') || command.startsWith('kill') || command.startsWith('delete_process')) {
          await Processes.refresh();
          await Dashboard.refresh();
        } else if (command.startsWith('touch') || command.startsWith('rm') || command.startsWith('write') || command.startsWith('mkdir') || command.startsWith('rmdir')) {
          await Files.refresh();
          await Dashboard.refresh();
        } else if (command.startsWith('firstfit') || command.startsWith('bestfit') || command.startsWith('worstfit') || command.startsWith('free')) {
          await Memory.refresh();
          await Dashboard.refresh();
        } else if (command.startsWith('deadlock')) {
          await Deadlock.refresh();
          await Dashboard.refresh();
        }
      }
    } catch (err) {
      if (output) {
        const errBlock = document.createElement('div');
        errBlock.className = 'terminal-line-error';
        errBlock.textContent = `[Error executing command: ${err.message}]`;
        output.appendChild(errBlock);
        output.scrollTop = output.scrollHeight;
      }
    } finally {
      this.isExecuting = false;
      if (input) input.focus();
    }
  },

  clearTerminal() {
    const output = document.getElementById('terminal-output');
    if (output) {
      output.innerHTML = '';
      this.printBanner();
    }
    const input = document.getElementById('terminal-input');
    if (input) input.focus();
  },

  escapeHtml(str) {
    return str
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;');
  }
};

