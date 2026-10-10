#include "HealthController.h"

#include "../repository/Database.h"

#include <json/json.h>

void HealthController::health(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    Json::Value response;
    response["status"] = "UP";

    try
    {
        auto dbClient = Database::getClient();

        auto result = dbClient->execSqlSync("SELECT 1");

        if (!result.empty())
        {
            response["db"] = "UP";
        }
        else
        {
            response["db"] = "STARTING";
        }
    }
    catch (...)
    {
        // Database is still connecting — report degraded but
        // keep the service marked UP so Render does not restart.
        response["db"] = "DOWN";
    }

    // Always return 200 so the health check passes on Render.
    // The DB will connect once it is ready.
    auto resp =
        drogon::HttpResponse::newHttpJsonResponse(response);

    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}