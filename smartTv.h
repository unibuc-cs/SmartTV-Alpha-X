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

    class SmartTv
    {

    private:
        time_point<high_resolution_clock> start_time;
        time_point<high_resolution_clock> last_time;
        int idle_duration = 10000;

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