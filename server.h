#pragma once
#include <algorithm>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>
#include <signal.h>

#include "endpoints.h"
#include "server.h"

using namespace std;
using namespace Pistache;
using namespace EndpointsN;

namespace ServerN
{    
    class Server 
    {
    public:
        void init();
    };

    void Server::init() 
    {
        sigset_t signals;
        if (sigemptyset(&signals) != 0
                || sigaddset(&signals, SIGTERM) != 0
                || sigaddset(&signals, SIGINT) != 0
                || sigaddset(&signals, SIGHUP) != 0
                || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) {
            perror("install signal handler failed");
            exit(1);
        }

        Port port(9080);

        int thr = 2;

        Address addr(Ipv4::any(), port);

        cout << "Server starting ... " << endl;
        cout << "Server run on localhost:9080 " << endl;

        Endpoints stats(addr);

        stats.init(thr);
        stats.start();

        int signal = 0;
        int status = sigwait(&signals, &signal);
        if (status == 0)
        {
            std::cout << std::endl << "Server closed" << std::endl;
        }
        else
        {
            std::cerr << "sigwait returns " << status << std::endl;
        }

        stats.stop();
    }
}