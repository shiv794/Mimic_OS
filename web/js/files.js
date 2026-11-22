/**
 * Mimic OS - File Management Controller
 * Educational Virtual File System Visualizer
 */
const Files = {
  currentFiles: [],
  currentDirectories: [],
  selectedFile: null,
  selectedDirectory: "/",
  _editorDirty: false,   // true while user has unsaved edits

  init() {
    const formCreateFile = document.getElementById('form-create-file');
    if (formCreateFile) {
      formCreateFile.addEventListener('submit', (e) => this.handleCreateFile(e));
    }

    const formCreateDir = document.getElementById('form-create-dir');
    if (formCreateDir) {
      formCreateDir.addEventListener('submit', (e) => this.handleCreateDirectory(e));
    }

    const btnSaveContent = document.getElementById('btn-save-file-content');
    if (btnSaveContent) {
      btnSaveContent.addEventListener('click', () => this.handleSaveContent());
    }

    const btnDeleteCurrent = document.getElementById('btn-delete-active-file');
    if (btnDeleteCurrent) {
      btnDeleteCurrent.addEventListener('click', () => {
        if (this.selectedFile) {
          this.deleteFile(this.selectedFile.name);
        }
      });
    }

    const btnResetFS = document.getElementById('btn-reset-fs');
    if (btnResetFS) {
      btnResetFS.addEventListener('click', () => this.resetFileSystem());
    }

    // --- FIX: track user typing so background refresh never wipes it ---
    // Use event delegation on the document (textarea may not exist yet at init time)
    document.addEventListener('input', (e) => {
      if (e.target && e.target.id === 'editor-file-content') {
        this._editorDirty = true;
        // Keep selectedFile.content in sync with what the user is typing
        // so when they hit Save it uses the latest value
        if (this.selectedFile) {
          this.selectedFile.content = e.target.value;
        }
      }
    });
  },

  async refresh() {
    try {
      const data = await API.getFiles();
      const newFiles = data.files || [];

      // --- FIX: If user has unsaved edits, preserve their in-memory content
      // so the background refresh does not overwrite it in the file list either.
      if (this.selectedFile && this._editorDirty) {
        const liveContent = this.selectedFile.content; // the value user is typing
        const match = newFiles.find(f => f.name === this.selectedFile.name && (f.directory || "/") === (this.selectedFile.directory || "/"));
        if (match) {
          // Restore the draft so selectedFile still points to the live copy
          match.content = liveContent;
        }
      }

      this.currentFiles = newFiles;
      this.currentDirectories = data.directories || [];

      // Re-point selectedFile to its refreshed object (so size etc. update)
      if (this.selectedFile) {
        const updated = this.currentFiles.find(f => f.name === this.selectedFile.name && (f.directory || "/") === (this.selectedFile.directory || "/"));
        if (updated) {
          // Preserve content if dirty
          if (this._editorDirty) updated.content = this.selectedFile.content;
          this.selectedFile = updated;
        } else {
          // File was deleted externally
          this.selectedFile = null;
          this._editorDirty = false;
        }
      }

      this.render();
    } catch (err) {
      console.warn('[Files] Refresh failed:', err.message);
    }
  },

  render() {
    // 1. Render Directories Panel
    const dirList = document.getElementById('dir-tree-list');
    const badgeDirCount = document.getElementById('badge-dir-count');
    if (badgeDirCount) badgeDirCount.textContent = this.currentDirectories.length;

    if (dirList) {
      if (this.currentDirectories.length === 0) {
        dirList.innerHTML = `<div class="empty-list-msg">No directories found</div>`;
      } else {
        dirList.innerHTML = this.currentDirectories.map(dir => {
          const isActive = this.selectedDirectory === dir.name;
          const isRoot = dir.name === "/";
          return `
            <div class="dir-item ${isActive ? 'active' : ''}" onclick="Files.selectDirectory('${dir.name}')">
              <span class="dir-icon">📁</span>
              <span class="dir-name">${dir.name}</span>
              ${!isRoot ? `
                <button class="btn-icon-danger" onclick="event.stopPropagation(); Files.deleteDirectory('${dir.name}')" title="Delete directory">
                  ✕
                </button>
              ` : '<span class="root-tag">ROOT</span>'}
            </div>
          `;
        }).join('');
      }
    }

    // 2. Render File List
    const tbody = document.getElementById('tbody-files');
    const badgeFileCount = document.getElementById('badge-file-count');
    
    // Filter files for the currently selected directory
    const currentDir = this.selectedDirectory || "/";
    const dirFiles = this.currentFiles.filter(f => (f.directory || "/") === currentDir);

    if (badgeFileCount) badgeFileCount.textContent = dirFiles.length;

    if (tbody) {
      if (dirFiles.length === 0) {
        tbody.innerHTML = `<tr><td colspan="5" style="text-align:center; color:var(--text-dim); padding:2rem;">No files in <code>${currentDir}</code>. Create one above.</td></tr>`;
      } else {
        tbody.innerHTML = dirFiles.map(f => {
          const isSelected = this.selectedFile && this.selectedFile.name === f.name && (this.selectedFile.directory || "/") === (f.directory || "/");
          return `
            <tr class="${isSelected ? 'row-selected' : ''}" onclick="Files.selectFile('${f.name}', '${f.directory || '/'}')" style="cursor:pointer;">
              <td>
                <span class="file-table-icon">📄</span>
                <strong>${f.name}</strong>
              </td>
              <td><code>${f.size} bytes</code></td>
              <td><code>${f.directory || '/'}</code></td>
              <td>
                <button class="btn btn-outline btn-sm" onclick="event.stopPropagation(); Files.selectFile('${f.name}', '${f.directory || '/'}')">
                  View / Edit
                </button>
                <button class="btn btn-danger btn-sm" onclick="event.stopPropagation(); Files.deleteFile('${f.name}', '${f.directory || '/'}')" style="margin-left:4px;">
                  ✕
                </button>
              </td>
            </tr>
          `;
        }).join('');
      }
    }

    // 3. Render Editor / Details Panel
    this.renderEditor();
  },

  selectDirectory(dirName) {
    this.selectedDirectory = dirName;
    // If the currently open file is not in this directory, clear selectedFile or keep it if desired
    if (this.selectedFile && (this.selectedFile.directory || "/") !== dirName) {
      if (!this._editorDirty) {
        this.selectedFile = null;
      }
    }
    this.render();
  },

  selectFile(fileName, dirName) {
    const targetDir = dirName || this.selectedDirectory || "/";
    const file = this.currentFiles.find(f => f.name === fileName && (f.directory || "/") === targetDir);
    if (file) {
      this.selectedFile = file;
      this.selectedDirectory = file.directory || "/";
      this._editorDirty = false; // Switching file clears any pending draft
      this.render();
    }
  },

  renderEditor() {
    const nameEl = document.getElementById('editor-file-name');
    const sizeEl = document.getElementById('editor-file-size');
    const contentTextarea = document.getElementById('editor-file-content');
    const actionsBox = document.getElementById('editor-actions-box');
    const noFileMsg = document.getElementById('editor-no-file-msg');

    if (!this.selectedFile) {
      if (nameEl) nameEl.textContent = "No File Selected";
      if (sizeEl) sizeEl.textContent = "--";
      if (contentTextarea) {
        contentTextarea.value = "";
        contentTextarea.disabled = true;
      }
      if (actionsBox) actionsBox.style.display = "none";
      if (noFileMsg) noFileMsg.style.display = "block";
      return;
    }

    if (nameEl) nameEl.textContent = this.selectedFile.name;
    if (sizeEl) sizeEl.textContent = `${this.selectedFile.size} bytes`;

    if (contentTextarea) {
      contentTextarea.disabled = false;

      // --- FIX: Never overwrite content while the user is actively typing ---
      // Only update the textarea value if:
      //  (a) the textarea is NOT focused (user is not typing), AND
      //  (b) the editor is not in a dirty/unsaved state
      const isFocused = (document.activeElement === contentTextarea);
      if (!isFocused && !this._editorDirty) {
        contentTextarea.value = this.selectedFile.content;
      }
    }

    if (actionsBox) actionsBox.style.display = "flex";
    if (noFileMsg) noFileMsg.style.display = "none";
  },

  async handleCreateFile(e) {
    e.preventDefault();
    const input = document.getElementById('input-create-filename');
    const name = input ? input.value.trim() : "";
    const dir = this.selectedDirectory || "/";

    if (!name) {
      App.showToast("Please enter a file name", "error");
      return;
    }

    try {
      const res = await API.createFile(name, dir);
      if (res.success) {
        App.showToast(`File '${name}' created in ${dir}`, "success");
        if (input) input.value = "";
        await this.refresh();
        this._editorDirty = false;
        this.selectFile(name, dir);
        await Dashboard.refresh();
      } else {
        App.showToast(res.message || "Failed to create file", "error");
      }
    } catch (err) {
      App.showToast("Failed to create file", "error");
    }
  },

  async deleteFile(name, dir) {
    if (!name) return;
    const targetDir = dir || (this.selectedFile ? this.selectedFile.directory : this.selectedDirectory) || "/";
    try {
      const res = await API.deleteFile(name, targetDir);
      if (res.success) {
        App.showToast(`File '${name}' deleted`, "info");
        if (this.selectedFile && this.selectedFile.name === name && (this.selectedFile.directory || "/") === targetDir) {
          this.selectedFile = null;
        }
        await this.refresh();
        await Dashboard.refresh();
      } else {
        App.showToast(res.message || "Delete failed", "error");
      }
    } catch (err) {
      App.showToast("Failed to delete file", "error");
    }
  },

  async handleSaveContent() {
    if (!this.selectedFile) return;

    const textarea = document.getElementById('editor-file-content');
    const content = textarea ? textarea.value : "";
    const dir = this.selectedFile.directory || "/";

    try {
      const res = await API.writeFile(this.selectedFile.name, content, dir);
      if (res.success) {
        App.showToast(`Changes saved to '${this.selectedFile.name}' (${content.length} bytes)`, "success");
        this.selectedFile.content = content;
        this.selectedFile.size = content.length;
        this._editorDirty = false; // Mark as clean after save
        await this.refresh();
        await Dashboard.refresh();
      } else {
        App.showToast(res.message || "Write failed", "error");
      }
    } catch (err) {
      App.showToast("Failed to save content", "error");
    }
  },

  async handleCreateDirectory(e) {
    e.preventDefault();
    const input = document.getElementById('input-create-dirname');
    let name = input ? input.value.trim() : "";

    if (!name) {
      App.showToast("Please enter a directory name", "error");
      return;
    }

    if (!name.startsWith("/")) {
      name = "/" + name;
    }

    try {
      const res = await API.createDirectory(name);
      if (res.success) {
        App.showToast(`Directory '${name}' created`, "success");
        if (input) input.value = "";
        await this.refresh();
        await Dashboard.refresh();
      } else {
        App.showToast(res.message || "Failed to create directory", "error");
      }
    } catch (err) {
      App.showToast("Failed to create directory", "error");
    }
  },

  async deleteDirectory(name) {
    try {
      const res = await API.deleteDirectory(name);
      if (res.success) {
        App.showToast(`Directory '${name}' deleted`, "info");
        if (this.selectedDirectory === name) {
          this.selectedDirectory = "/";
        }
        await this.refresh();
        await Dashboard.refresh();
      } else {
        App.showToast(res.message || "Delete failed", "error");
      }
    } catch (err) {
      App.showToast("Failed to delete directory", "error");
    }
  },

  async resetFileSystem() {
    try {
      await API.resetFiles();
      App.showToast("Virtual File System reset to default state", "info");
      this.selectedFile = null;
      await this.refresh();
      await Dashboard.refresh();
    } catch (err) {
      App.showToast("Reset failed", "error");
    }
  }
};

