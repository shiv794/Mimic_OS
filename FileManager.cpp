#include "FileManager.h"
#include <iostream>

using namespace std;

FileManager::FileManager() {
    reset();
}

void FileManager::reset() {
    files.clear();
    directories.clear();

    // Default directories
    Directory root; root.name = "/"; directories.push_back(root);
    Directory bin; bin.name = "/bin"; directories.push_back(bin);
    Directory docs; docs.name = "/docs"; directories.push_back(docs);
    Directory home; home.name = "/home"; directories.push_back(home);

    // Default sample files for education placed in proper directories
    File f1;
    f1.name = "readme.txt";
    f1.directory = "/";
    f1.content = "Welcome to Mimic OS Virtual File System!\nThis is an educational OS simulator demonstrating file management.";
    f1.size = (int)f1.content.length();
    files.push_back(f1);

    File f2;
    f2.name = "kernel.log";
    f2.directory = "/bin";
    f2.content = "[INFO] Mimic OS Kernel initialized.\n[INFO] CPU and Memory Manager online.\n[INFO] Virtual File System mounted successfully.";
    f2.size = (int)f2.content.length();
    files.push_back(f2);

    File f3;
    f3.name = "notes.txt";
    f3.directory = "/docs";
    f3.content = "Operating System concepts:\n- Process Scheduling (FCFS, SJF, Priority, RR)\n- Memory Allocation (First, Best, Worst Fit)\n- Deadlock Detection (Banker's Algorithm)";
    f3.size = (int)f3.content.length();
    files.push_back(f3);
}

// CREATE FILE

void FileManager::createFile(string name, string directory) {
    if (directory.empty()) directory = "/";

    if (fileExists(name, directory)) {
        cout << "File already exists in directory " << directory << ".\n";
        return;
    }

    File newFile;
    newFile.name = name;
    newFile.directory = directory;
    newFile.content = "";
    newFile.size = 0;

    files.push_back(newFile);

    cout << "File '" << name << "' created successfully in " << directory << ".\n";
}

// DELETE FILE

void FileManager::deleteFile(string name, string directory) {
    for (size_t i = 0; i < files.size(); i++) {
        if (files[i].name == name && (directory.empty() || files[i].directory == directory)) {
            files.erase(files.begin() + i);
            cout << "File '" << name << "' deleted successfully.\n";
            return;
        }
    }

    cout << "File not found.\n";
}

// WRITE FILE

void FileManager::writeFile(string name, string content, string directory) {
    if (directory.empty()) directory = "/";

    for (File& file : files) {
        if (file.name == name && (file.directory == directory || directory.empty())) {
            file.content = content;
            file.size = (int)content.length();
            cout << "Content written to '" << name << "'.\n";
            return;
        }
    }

    // Auto-create file if it does not exist
    File newFile;
    newFile.name = name;
    newFile.directory = directory;
    newFile.content = content;
    newFile.size = (int)content.length();
    files.push_back(newFile);
    cout << "File '" << name << "' created in " << directory << " and content written.\n";
}

// READ FILE

void FileManager::readFile(string name, string directory) const {
    for (const File& file : files) {
        if (file.name == name && (directory.empty() || file.directory == directory)) {
            cout << "\nFILE CONTENT\n";
            cout << "File: " << file.name << endl;
            cout << "Directory: " << file.directory << endl;
            cout << "Size: " << file.size << " bytes\n";
            cout << "Content:\n";
            cout << file.content << endl;
            cout << "\n";
            return;
        }
    }

    cout << "File not found.\n";
}

// CREATE DIRECTORY

void FileManager::createDirectory(string name) {
    if (directoryExists(name)) {
        cout << "Directory already exists.\n";
        return;
    }

    Directory directory;
    directory.name = name;
    directories.push_back(directory);

    cout << "Directory '" << name << "' created successfully.\n";
}

// DELETE DIRECTORY

void FileManager::deleteDirectory(string name) {
    if (name == "/") {
        cout << "Root directory cannot be deleted.\n";
        return;
    }

    for (size_t i = 0; i < directories.size(); i++) {
        if (directories[i].name == name) {
            directories.erase(directories.begin() + i);

            // Also delete files in this directory
            for (int f = (int)files.size() - 1; f >= 0; f--) {
                if (files[f].directory == name) {
                    files.erase(files.begin() + f);
                }
            }

            cout << "Directory '" << name << "' deleted successfully.\n";
            return;
        }
    }

    cout << "Directory not found.\n";
}

// LIST FILES

void FileManager::listFiles(string directory) const {
    cout << "\nFILES" << (directory.empty() ? "" : " in " + directory) << "\n";

    bool found = false;
    for (const File& file : files) {
        if (directory.empty() || file.directory == directory) {
            cout << file.name << " (" << file.size << " bytes) [" << file.directory << "]\n";
            found = true;
        }
    }

    if (!found) {
        cout << "No files.\n";
    }
    cout << "\n";
}

// LIST DIRECTORIES

void FileManager::listDirectories() const {
    cout << "\nDIRECTORIES\n";
    for (const Directory& directory : directories) {
        cout << directory.name << endl;
    }
    cout << "\n";
}

// FILE EXISTS

bool FileManager::fileExists(string name, string directory) const {
    for (const File& file : files) {
        if (file.name == name && (directory.empty() || file.directory == directory))
            return true;
    }
    return false;
}

// DIRECTORY EXISTS

bool FileManager::directoryExists(string name) const {
    for (const Directory& directory : directories) {
        if (directory.name == name)
            return true;
    }
    return false;
}

const File* FileManager::getFile(const string& name, const string& directory) const {
    for (const File& file : files) {
        if (file.name == name && (directory.empty() || file.directory == directory))
            return &file;
    }
    return nullptr;
}

string FileManager::getFileContent(const string& name, const string& directory) const {
    const File* f = getFile(name, directory);
    if (f) return f->content;
    return "";
}
