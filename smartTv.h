#pragma once
#include <algorithm>
#include <stdio.h>

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

namespace SmartTvN {

    void getTest(const Rest::Request &request, Http::ResponseWriter response) {
        auto param = request.param(":var").as<int>();
        response.send(Http::Code::Ok, param.as<string>());
    }
}