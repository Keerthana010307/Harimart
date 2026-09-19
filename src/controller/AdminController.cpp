#include "AdminController.h"
#include "../util/AdminAuthUtil.h"
#include "../repository/Database.h"

void AdminController::getUsers(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    if (!isAdmin(req))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Admin access required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id, name, email, role "
        "FROM users "
        "ORDER BY id"
    );

    Json::Value users(Json::arrayValue);

    for (const auto& row : result)
    {
        Json::Value user;
        user["id"] = Json::Int64(row["id"].as<long long>());
        user["name"] = row["name"].as<std::string>();
        user["email"] = row["email"].as<std::string>();
        user["role"] = row["role"].as<std::string>();

        users.append(user);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["users"] = users;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}
