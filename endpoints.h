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
#include <nlohmann/json.hpp>

using namespace std;
using namespace Pistache;
using json = nlohmann::json;

namespace EndpointsN {

    class Endpoints {
    public:
    
        using Lock = std::mutex;
        using Guard = std::lock_guard<Lock>;
        Lock lock;

        std::shared_ptr<Http::Endpoint> httpEndpoint;
        Rest::Router router;
        
        explicit Endpoints(Address addr)
            : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
        { }

        void init(size_t);

        void start();

        void stop();

    private:
        void setupRoutes() {
            using namespace Rest;

            // curl localhost:9080/test/stringTest
            Routes::Get(router, "/test/:var", Routes::bind(&Endpoints::getTest, this));

            // curl localhost:9080/modificare-luminozitate/100
            Routes::Get(router, "/modificare-luminozitate/:var", Routes::bind(&Endpoints::getLuminozitate, this));

            // curl -X POST -H "Content-Type: application/json" -d '{"id":1,"name":"test"}' localhost:9080/test
            Routes::Post(router, "/test", Routes::bind(&Endpoints::postTest, this));

            // curl -X POST -H "Content-Type: application/json" -d '{"name":"modificare-luminozitate", "value":"100"}' localhost:9080/modificare-luminozitate
            Routes::Post(router, "/modificare-luminozitate", Routes::bind(&Endpoints::postLuminozitate, this));
        }

        void getTest(const Rest::Request&, Http::ResponseWriter);
        void getLuminozitate(const Rest::Request&, Http::ResponseWriter);
        void postTest(const Rest::Request&, Http::ResponseWriter);
        void postLuminozitate(const Rest::Request&, Http::ResponseWriter);

    };


    // IMPLEMENTATIONS
    void Endpoints::init(size_t thr = 2) 
    {
        auto opts = Http::Endpoint::options()
            .threads(static_cast<int>(thr));
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void Endpoints::start() 
    {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
    }

    void Endpoints::stop() 
    {
        httpEndpoint->shutdown();
    }

    void Endpoints::getTest(const Rest::Request& request, Http::ResponseWriter response)
    {
        auto param = request.param(":var").as<string>();
        response.send(Http::Code::Ok, param);
    }

    void Endpoints::getLuminozitate(const Rest::Request& request, Http::ResponseWriter response)
    {
        auto param = request.param(":var").as<string>();
        response.send(Http::Code::Ok, param);
    }

    void Endpoints::postTest(const Rest::Request& request, Http::ResponseWriter response)
    {
        json j;
        j["hardcoded"] = R"(
            {
                "happy": true,
                "pi": 3.141
            }
        )"_json;
        auto bodyJson = json::parse(request.body());
        j["received"] = bodyJson;
        response.send(Http::Code::Ok, j.dump());
    }
    void Endpoints::postLuminozitate(const Rest::Request& request, Http::ResponseWriter response)
    {
        json j;
        j["hardcoded"] = R"(
            {
                "happy": true,
                "pi": 3.141
            }
        )"_json;
        auto bodyJson = json::parse(request.body());
        j["received"] = bodyJson;
        response.send(Http::Code::Ok, j.dump());
    }

}