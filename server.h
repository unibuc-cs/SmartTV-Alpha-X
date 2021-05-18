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
using namespace SmartTvN;

namespace ServerN
{    
    class Server 
    {
    public:
        void init();
        static void check_time(Endpoints *stats);
        static void signal_wait(int s);
    };

    void Server::check_time(Endpoints *stats){
        while(stats->smartTv.getTimeFromLast()<stats->smartTv.getIdleDuration())
        {
        }
    }

    void Server::signal_wait(int s){
           std::cout << std::endl << "Server closed manually" << std::endl;
           exit(1);
        }
  
    void Server::init() 
    {
        

        Port port(9080);

        int thr = 2;

        Address addr(Ipv4::any(), port);

        cout << "Server starting ... " << endl;
        cout << "Server run on localhost:9080 " << endl;

        Endpoints stats(addr);

        stats.init(thr);

        stats.start();

        thread t1(check_time, &stats);

        int x;
        struct sigaction sigIntHandler;

        sigIntHandler.sa_handler = signal_wait;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;

        sigaction(SIGINT, &sigIntHandler, NULL);

        t1.join();

        stats.stop();

        std::cout << std::endl << "Server closed after too much idle time" << std::endl;
    }
}