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
#include <mosquitto.h>

#include "endpoints.h"
#include "server.h"

using namespace std;
using namespace Pistache;
using namespace EndpointsN;
using namespace SmartTvN;
using json = nlohmann::json;

namespace ServerN
{    
    class Server 
    {
    public:
        void init();
        static void on_connect(struct mosquitto *mosq, void *obj, int rc);
        static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);
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

        int thr = 2, rc, id = 12345;

        Address addr(Ipv4::any(), port);

        cout << "Server starting ... " << endl;
        cout << "Server run on localhost:9080 " << endl;

        Endpoints stats(addr);

        stats.init(thr);

        mosquitto_lib_init();
	    struct mosquitto *mosq;
        mosq = mosquitto_new("subscribe-test", true, &id);
        mosquitto_connect_callback_set(mosq, Server::on_connect);
        mosquitto_message_callback_set(mosq, Server::on_message);
        rc = mosquitto_connect(mosq, "localhost", 1883, 10);
        if(rc) {
            std::cout << "Could not connect to Broker with return code " << rc << "\n";
        }
	    mosquitto_loop_start(mosq);

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

        mosquitto_loop_stop(mosq, true);
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();

        std::cout << std::endl << "Server closed after too much idle time" << std::endl;
    }

    
    void Server::on_connect(struct mosquitto *mosq, void *obj, int rc) {
        if(rc) {
            std::cout << "Error with result code: " << rc <<  "\n";
        }

        // add subscription foreach route
        mosquitto_subscribe(mosq, NULL, "test/t1", 0);
    }

    void Server::on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
        string s = (char *) msg->payload;
        // mosquitto_pub -m "{\"id\":1,\"name\":\"test\"}" -t "test/t1"
        std::cout << "Topic: "  << msg->topic << " Message: " << s << '\n';  
        auto json = json::parse(s);
        cout << "ID: " << json["id"] << " Name " << json["name"] << '\n';

        // if (msg->topic === "test/t1") call method for this route
    }

}