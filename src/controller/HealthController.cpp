#include "HealthController.h"

#include "../repository/Database.h"

#include <json/json.h>

void HealthController::health(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    Json::Value response;

    try
    {
        auto dbClient = Database::getClient();

        auto result = dbClient->execSqlSync("SELECT 1");

        if (!result.empty())
        {
            response["status"] = "UP";
            response["db"] = "UP";

            auto resp =
                drogon::HttpResponse::newHttpJsonResponse(response);

            resp->setStatusCode(drogon::k200OK);
            callback(resp);
            return;
        }
    }
    catch (...)
    {
        // Database is unavailable
    }

    response["status"] = "UP";
    response["db"] = "DOWN";

    auto resp =
        drogon::HttpResponse::newHttpJsonResponse(response);

    resp->setStatusCode(drogon::k503ServiceUnavailable);
    callback(resp);
}