#include <iostream>
#include <windows.h>
#include <shellapi.h>
#include "HttpServer.h"

using namespace std;

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        try {
            port = stoi(argv[1]);
        } catch (...) {
            port = 8080;
        }
    }

    cout << "Starting Mimic OS Web GUI..." << endl;

    // Automatically open browser after 500ms
    string url = "http://localhost:" + to_string(port);
    ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);

    HttpServer server(port, "./web");
    if (!server.start()) {
        cerr << "Failed to start Mimic OS server." << endl;
        return 1;
    }

    return 0;
}

