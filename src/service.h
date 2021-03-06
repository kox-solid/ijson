
#pragma once

#include <thread>
#include "utils.h"

class Server;


class Service {
private:
    std::thread _thread;
    Server *server;
    void _start();
    void _free_memory();
    void _clean_dead_tasks();
public:
    Service(Server *server) : server(server) {};
    void start();
};
