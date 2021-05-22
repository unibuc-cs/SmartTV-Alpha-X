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
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <map>

using namespace std;
using namespace Pistache;
using namespace SmartTvN;
using json = nlohmann::json;


const int threshold_TIME = 120;

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

            // curl localhost:9080/timp-start - to be renamed
            Routes::Get(router, "/timp-start", Routes::bind(&Endpoints::getTimeFromStart, this));

            // curl localhost:9080/timp-last - to be renamed
            Routes::Get(router, "/timp-last", Routes::bind(&Endpoints::getTimeFromLast, this));

            // curl localhost:9080/getUsers
            Routes::Get(router, "/getUsers", Routes::bind(&Endpoints::getUsers, this));

            // curl -X POST localhost:9080/timp-idle/20 - to be renamed
            Routes::Post(router, "/timp-idle/:time", Routes::bind(&Endpoints::postIdleTime, this));
        
            // curl -X GET localhost:9080/getSuggestedChannels/Muzica/20
            Routes::Get(router, "/getSuggestedChannels/:gen/:varsta", Routes::bind(&Endpoints::getChannels, this));

            // curl -X GET localhost:9080/getHistoryAndRecommandations/anca
            Routes::Get(router, "/getHistoryAndRecommandations/:nume", Routes::bind(&Endpoints::getHistoryAndRecommandations, this));

            // curl -X POST localhost:9080/insertUser/flavius/20
            Routes::Post(router, "/insertUser/:username/:varsta", Routes::bind(&Endpoints::insertUser, this));

            // curl -X POST localhost:9080/addChannel/anca/KissTV
            Routes::Post(router, "/addChannel/:username/:canal", Routes::bind(&Endpoints::addChannelToUser, this));

            // curl -X POST localhost:9080/setBrightness/20
            Routes::Post(router, "/setBrightness/:level", Routes::bind(&Endpoints::setBrightness, this));

            //curl -X GET localhost:9080/notificationDistance/75/1.75
            Routes::Get(router,"/notificationDistance/:size/:current_distance", Routes::bind(&Endpoints::setNotification, this) );
        }

        void getTimeFromStart(const Rest::Request&, Http::ResponseWriter);
        void getTimeFromLast(const Rest::Request&, Http::ResponseWriter);
        void postIdleTime(const Rest::Request&, Http::ResponseWriter);
        void getChannels(const Rest::Request&, Http::ResponseWriter);
        void getHistoryAndRecommandations(const Rest::Request&, Http::ResponseWriter);
        void insertUser(const Rest::Request&, Http::ResponseWriter);
        void editDataUser(const Rest::Request&, Http::ResponseWriter);
        void addChannelToUser(const Rest::Request&, Http::ResponseWriter);
        void getUsers(const Rest::Request&, Http::ResponseWriter);

        void setBrightness(const Rest::Request&, Http::ResponseWriter);
        void setNotification(const Rest::Request&, Http::ResponseWriter);


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


        vector<string> suggestions = smartTv.getSuggestions(gen, varsta);

        string output = "";
        for(int i = 0; i < suggestions.size(); i++){
            output += suggestions[i] + ',';
        }

        json j = {
            {"channels", output}
        };
        response.send(Http::Code::Ok, j.dump());

    }

    void Endpoints::getHistoryAndRecommandations(const Rest::Request& request, Http::ResponseWriter response){
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        auto name = request.param(":nume").as<string>();


        map<std::string, int> genres_user = smartTv.getGenres(name);

        vector<std::pair<std::string, std::string>> genres_rec = smartTv.getGenRec(name);

        json j = {
            {"istoric", genres_user},
            {"canale recomandate", genres_rec}
        };
        response.send(Http::Code::Ok, j.dump());

    }

    void Endpoints::insertUser(const Rest::Request& request, Http::ResponseWriter response){
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        auto username = request.param(":username").as<string>();
        auto varsta = request.param(":varsta").as<int>();

        vector<User*> users = smartTv.getUsers();
        bool ok = 0;
        for(int i  = 0; i < users.size(); i++){
            if(username == users[i]->getUsername()){
                response.send(Http::Code::Bad_Request, "The user already added.");
                ok = 1;
            }
        }

        if(ok == 0){
            smartTv.add_user(username, varsta);
            response.send(Http::Code::Ok);
        }

    }


    void Endpoints::getUsers(const Rest::Request& request, Http::ResponseWriter response){
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));

        vector<User*> users = smartTv.getUsers();
        json json_array = json::array();
        for(int i = 0; i < users.size(); i++){
            vector<Channel*> channels = users[i]->getListaCanale();

            string canale = "";
            for(int j = 0; j < channels.size(); j++){
                canale += channels[i]->getNume() + ",";
            }
            json j = {
                {"nume", users[i]->getUsername()},
                {"varsta", users[i]->getVarsta()},
                {"channels", canale}
            };
            json_array.push_back(j);
        }

        response.send(Http::Code::Ok, json_array.dump());
    }


    void Endpoints::addChannelToUser(const Rest::Request& request, Http::ResponseWriter response){
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        auto username = request.param(":username").as<string>();
        auto canal = request.param(":canal").as<string>();

        vector<User*> users = smartTv.getUsers();
        bool exist_user = 0;
        int pos_user = -1;
        for(int i  = 0; i < users.size(); i++){
            if(username == users[i]->getUsername()){
                exist_user = 1;
                pos_user = i;
            }
        }

        std::ofstream file("text.out");
        file << exist_user;
        file.close();

        if(exist_user == 0){
            response.send(Http::Code::Bad_Request, "The user does not exist.");
        }

        bool exist_channel = 0;
        vector<Channel*> channels = smartTv.getChannels();
        for(int i = 0; i < channels.size(); i++){
            if(canal == channels[i]->getNume()){
                exist_channel = 1;
            }
        }

        if(exist_channel == 0){
            response.send(Http::Code::Bad_Request, "The channel does not exist.");
        }

        if(exist_user){
            vector<Channel*> channels_user = users[pos_user]->getListaCanale();
            bool channel_added = 0;
            for(int i = 0; i < channels_user.size(); i++){
                if(channels_user[i]->getNume() == canal){
                    response.send(Http::Code::Bad_Request, "The channel was already added to user's channel list.");
                    channel_added = 1;
                }
            }
            if (channel_added == 0){
                smartTv.add_channel_to_user(username, canal);
                response.send(Http::Code::Ok);
            }
        }

    }

    
    
    void Endpoints::setBrightness(const Rest::Request& request, Http::ResponseWriter response)
    {   
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        auto level = request.param(":level").as<int>();
        
        smartTv.setBrightness(level);
         json j = {
            {"new_brightness", smartTv.getBrightness()}
        };
        response.send(Http::Code::Ok, j.dump());
     
    }


    void Endpoints::setNotification(const Rest::Request& request, Http::ResponseWriter response)
        {   
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            auto size = request.param(":size").as<int>();
            auto current_distance = request.param(":current_distance").as<float>();
            
            string message = smartTv.notifyUserDistance(size, current_distance);
         
            json j = {
                {"notification_message", message}
            };
            response.send(Http::Code::Ok, j.dump());
        
        }

}