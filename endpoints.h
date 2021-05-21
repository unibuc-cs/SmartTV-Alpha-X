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
#include "smartTv.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace Pistache;
using namespace SmartTvN;
using json = nlohmann::json;

namespace EndpointsN {

    class Endpoints {
    public:
    
        using Lock = std::mutex;
        using Guard = std::lock_guard<Lock>;
        Lock lock;

        std::shared_ptr<Http::Endpoint> httpEndpoint;
        Rest::Router router;

        static SmartTv smartTv;
        
        explicit Endpoints(Address addr)
            : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
        { }

        void init(size_t);

        void start();

        void stop();
    
    private:
        void setupRoutes() {
            using namespace Rest;

            // curl localhost:9080/timp-start
            Routes::Get(router, "/timp-start", Routes::bind(&Endpoints::getTimeFromStart, this));

            // curl localhost:9080/timp-last
            Routes::Get(router, "/timp-last", Routes::bind(&Endpoints::getTimeFromLast, this));

            // curl -X POST localhost:9080/timp-idle/20
            Routes::Post(router, "/timp-idle/:time", Routes::bind(&Endpoints::postIdleTime, this));
        
            // curl -X GET localhost:9080/sugestii/Muzica/20
            Routes::Get(router, "/sugestii/:gen/:varsta", Routes::bind(&Endpoints::getChannels, this));
        }
        void getTimeFromStart(const Rest::Request&, Http::ResponseWriter);
        void getTimeFromLast(const Rest::Request&, Http::ResponseWriter);
        void postIdleTime(const Rest::Request&, Http::ResponseWriter);
        void getChannels(const Rest::Request&, Http::ResponseWriter);

    };


    // IMPLEMENTATIONS
    
    SmartTv Endpoints::smartTv;

    void Endpoints::init(size_t thr = 2) 
    {
        auto opts = Http::Endpoint::options()
            .threads(static_cast<int>(thr));
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void Endpoints::start() 
    {
        smartTv = SmartTv();
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
    }

    void Endpoints::stop() 
    {
        httpEndpoint->shutdown();
    }

    void Endpoints::getTimeFromStart(const Rest::Request&, Http::ResponseWriter response)
    {
        smartTv.restartTimeFromLast();
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        int time = smartTv.getTimeFromStart();
        json j = {
            {"time", time}
        };
        response.send(Http::Code::Ok, j.dump());
    }

    void Endpoints::getTimeFromLast(const Rest::Request&, Http::ResponseWriter response)
    {   
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        int time = smartTv.getTimeFromLast();
        json j = {
            {"time", time}
        };
        response.send(Http::Code::Ok, j.dump());
        smartTv.restartTimeFromLast();
    }

    void Endpoints::postIdleTime(const Rest::Request& request, Http::ResponseWriter response)
    {   
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        auto time = request.param(":time").as<int>();
        smartTv.restartTimeFromLast();
        smartTv.setIdleDuration(time);
        response.send(Http::Code::Ok);
    }

    void Endpoints::getChannels(const Rest::Request& request, Http::ResponseWriter response){
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        auto gen = request.param(":gen").as<string>();
        auto varsta = request.param(":varsta").as<int>();

        std::ifstream file("dateTV.csv");
        std::string str;

        std::string outputData = "";
        while(std::getline(file, str)){
            stringstream s_stream(str);
            vector<string> result;
            while(s_stream.good()){
                string substr;
                getline(s_stream, substr, ',');
                result.push_back(substr);
            }
            if(result[2] == gen){
                std::string s = result[3];
                std::string delimiter = "-";
                std::string first_number = s.substr(0, s.find(delimiter));
                s.erase(0, s.find(delimiter) + delimiter.length());
                int first_no = std::stoi(first_number);
                int second_no = std::stoi(s);

                if(varsta >= first_no && varsta <= second_no){
                    outputData = outputData + result[1] + ",";
                }
            }
        }

        if(outputData[outputData.size() - 1] == ','){
            outputData.pop_back();
        }

        std::ifstream input_json("output_sugestii.json");
        json content_json;
        input_json >> content_json;
        auto& output = content_json["output_buffers"];

        output["value"] = outputData;
        input_json.close();

        std::ofstream output_json("output_sugestii.json");
        output_json << std::setw(4) << content_json << std::endl;
        output_json.close();

        response.send(Http::Code::Ok, content_json.dump());
    }

}