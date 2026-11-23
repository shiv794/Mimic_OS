#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <vector>
#include <string>

using namespace std;

struct File {
    string name;
    string directory;
    string content;
    int size;
};

struct Directory {
    string name;
};

class FileManager {
private:
    vector<File> files;
    vector<Directory> directories;

public:
    FileManager();

    void createFile(string name, string directory = "/");
    void deleteFile(string name, string directory = "/");

    void writeFile(string name, string content, string directory = "/");
    void readFile(string name, string directory = "/") const;

    void createDirectory(string name);
    void deleteDirectory(string name);

    void listFiles(string directory = "") const;
    void listDirectories() const;

    bool fileExists(string name, string directory = "/") const;
    bool directoryExists(string name) const;
    
    const vector<File>& getFiles() const { return files; }
    const vector<Directory>& getDirectories() const { return directories; }
    const File* getFile(const string& name, const string& directory = "/") const;
    string getFileContent(const string& name, const string& directory = "/") const;
    void reset();
};

#endif
