#pragma once

#include <drogon/HttpController.h>

class HealthController
    : public drogon::HttpController<HealthController>
{
public:

    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        HealthController::health,
        "/api/v1/health",
        drogon::Get
    );

    METHOD_LIST_END

    void health(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
};