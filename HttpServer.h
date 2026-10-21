#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include "ApiHandler.h"

using namespace std;

class HttpServer {
private:
    int port;
    string webRoot;
    bool running;
    ApiHandler apiHandler;

public:
    HttpServer(int port = 8080, const string& webRoot = "./web");
    ~HttpServer();

    bool start();
    void stop();
    int getPort() const { return port; }
};

#endif

