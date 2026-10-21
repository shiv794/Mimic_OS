#include "HttpServer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

static string getMimeType(const string& path) {
    size_t dotPos = path.find_last_of('.');
    if (dotPos == string::npos) return "text/plain";
    string ext = path.substr(dotPos);
    for (char& c : ext) c = tolower(c);

    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css") return "text/css; charset=utf-8";
    if (ext == ".js") return "application/javascript; charset=utf-8";
    if (ext == ".json") return "application/json; charset=utf-8";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".png") return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".ico") return "image/x-icon";
    if (ext == ".txt") return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

static bool readFileContents(const string& filePath, string& content) {
    ifstream file(filePath.c_str(), ios::in | ios::binary);
    if (!file.is_open()) return false;
    ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
}

HttpServer::HttpServer(int p, const string& root)
    : port(p), webRoot(root), running(false) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::stop() {
    running = false;
}

bool HttpServer::start() {
    WSADATA wsaData;
    int wsaRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaRes != 0) {
        cerr << "[HttpServer] WSAStartup failed: " << wsaRes << endl;
        return false;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "[HttpServer] socket creation failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return false;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        // Try fallback port 8081
        port++;
        serverAddr.sin_port = htons(port);
        if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "[HttpServer] bind failed on ports " << (port - 1) << " and " << port << endl;
            closesocket(listenSocket);
            WSACleanup();
            return false;
        }
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "[HttpServer] listen failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }

    running = true;
    cout << "\n======================================================\n";
    cout << "           MIMIC OS - INTERACTIVE WEB GUI SERVER       \n";
    cout << "   Educational Operating System Simulation Platform   \n";
    cout << "======================================================\n";
    cout << "[Server] Online on: http://localhost:" << port << endl;
    cout << "[Server] Serving GUI files from: " << webRoot << endl;
    cout << "[Server] Ready for web requests. Press Ctrl+C to stop.\n" << endl;

    while (running) {
        sockaddr_in clientAddr;
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == INVALID_SOCKET) {
            if (!running) break;
            continue;
        }

        // Read request headers
        char buffer[4096];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) {
            closesocket(clientSocket);
            continue;
        }
        buffer[bytesRead] = '\0';
        string requestStr(buffer, bytesRead);

        // Parse Request Line: METHOD PATH HTTP/VERSION
        istringstream reqStream(requestStr);
        string method, fullPath, httpVer;
        reqStream >> method >> fullPath >> httpVer;

        // Strip query string if any
        string path = fullPath;
        size_t queryPos = path.find('?');
        if (queryPos != string::npos) {
            path = path.substr(0, queryPos);
        }

        // Handle CORS Preflight
        if (method == "OPTIONS") {
            string response = "HTTP/1.1 200 OK\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                              "Access-Control-Allow-Headers: Content-Type\r\n"
                              "Content-Length: 0\r\n"
                              "Connection: close\r\n\r\n";
            send(clientSocket, response.c_str(), response.length(), 0);
            closesocket(clientSocket);
            continue;
        }

        // Extract Request Body if POST
        string body = "";
        if (method == "POST") {
            size_t headerEnd = requestStr.find("\r\n\r\n");
            if (headerEnd != string::npos) {
                body = requestStr.substr(headerEnd + 4);
            }

            // Check Content-Length to see if more body needs to be read
            size_t clPos = requestStr.find("Content-Length:");
            if (clPos == string::npos) clPos = requestStr.find("content-length:");
            if (clPos != string::npos) {
                size_t lineEnd = requestStr.find("\r\n", clPos);
                string clStr = requestStr.substr(clPos + 15, lineEnd - (clPos + 15));
                int expectedLen = 0;
                try { expectedLen = stoi(clStr); } catch (...) {}

                while ((int)body.length() < expectedLen) {
                    int more = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                    if (more <= 0) break;
                    buffer[more] = '\0';
                    body.append(buffer, more);
                }
            }
        }

        // Route: API Endpoint vs Static File
        string responseBody;
        string contentType = "text/html; charset=utf-8";
        int statusCode = 200;

        if (path.rfind("/api/", 0) == 0) {
            // Forward to API Handler
            apiHandler.handleRequest(method, path, body, responseBody, statusCode, contentType);
        } else {
            // Serve static file from webRoot
            if (path == "/" || path.empty()) {
                path = "/index.html";
            }

            // Security check: prevent ../ directory traversal
            if (path.find("..") != string::npos) {
                statusCode = 403;
                responseBody = "<h1>403 Forbidden</h1>";
                contentType = "text/html";
            } else {
                string filePath = webRoot + path;
                if (!readFileContents(filePath, responseBody)) {
                    // Try without leading slash or check fallback
                    statusCode = 404;
                    responseBody = "<h1>404 Not Found: " + path + "</h1>";
                    contentType = "text/html";
                } else {
                    contentType = getMimeType(filePath);
                }
            }
        }

        // Build HTTP Response
        string statusText = (statusCode == 200) ? "200 OK" :
                            (statusCode == 404) ? "404 Not Found" :
                            (statusCode == 403) ? "403 Forbidden" : "500 Internal Error";

        ostringstream resStream;
        resStream << "HTTP/1.1 " << statusText << "\r\n";
        resStream << "Content-Type: " << contentType << "\r\n";
        resStream << "Content-Length: " << responseBody.length() << "\r\n";
        resStream << "Access-Control-Allow-Origin: *\r\n";
        resStream << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        resStream << "Access-Control-Allow-Headers: Content-Type\r\n";
        resStream << "Connection: close\r\n";
        resStream << "\r\n";
        resStream << responseBody;

        string fullResponse = resStream.str();
        send(clientSocket, fullResponse.c_str(), fullResponse.length(), 0);
        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();
    return true;
}
