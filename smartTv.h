#pragma once
#include <algorithm>
#include <stdio.h>
#include <chrono>

#include <pistache/net.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/http_headers.h>
#include <pistache/cookie.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include <pistache/common.h>
#include <signal.h>

using namespace std;
using namespace Pistache;
using namespace chrono;

namespace SmartTvN
{
    class Channel{
        private:
            string nume;
            string gen;
            string intervalVarsta;
        
        public:
            string getNume(){
                return this->nume;
            }

            string getGen(){
                return this->gen;
            }

            string getIntervalVarsta(){
                return this->intervalVarsta;
            }

            void setNume(string nume){
                this->nume = nume;
            }

            void setGen(string gen){
                this->gen = gen;
            }

            void setIntervalVarsta(string interval){
                this->intervalVarsta = interval;
            }
    };

    class User{
        private:
            string username;
            int varsta;
            vector<Channel> listaCanale;
        
        public:
            string getUsername(){
                return this->username;
            }

            int getVarsta(){
                return this->varsta;
            }

            vector<Channel> getListaCanale(){
                return this->listaCanale;
            }

            void setUsername(string username){
                this->username = username;
            }

            void setVarsta(int varsta){
                this->varsta = varsta;
            }

            void setListaCanale(vector<Channel> canale){
                this->listaCanale = canale;
            }

            void addChannel(Channel canal){
                this->listaCanale.push_back(canal);
            }
    };

    class SmartTv
    {

    private:
        time_point<high_resolution_clock> start_time;
        time_point<high_resolution_clock> last_time;
        int idle_duration = 10000;
        vector<User> users;
        vector<Channel> channls;


    public:
        SmartTv();
        int getTimeFromStart();
        int getTimeFromLast();
        int getIdleDuration();
        void restartTimeFromLast();
        void setIdleDuration(int idle_duration);
    };

    // get the time (in seconds) since the TV has started
    int SmartTv::getTimeFromStart()
    {
        return duration_cast<seconds>(high_resolution_clock::now() - start_time).count();
    }

    // get the time (in seconds) since the last action on the TV
    int SmartTv::getTimeFromLast()
    {
        return duration_cast<seconds>(high_resolution_clock::now() - last_time).count();
    }

    void SmartTv::restartTimeFromLast()
    {
        last_time = high_resolution_clock::now();
    }

    void SmartTv::setIdleDuration(int idle_duration)
    {
        this->idle_duration = idle_duration;
    }
    int SmartTv::getIdleDuration()
    {
        return idle_duration;
    }

    SmartTv::SmartTv()
    {
        start_time = high_resolution_clock::now();
        last_time = high_resolution_clock::now();
    }

}