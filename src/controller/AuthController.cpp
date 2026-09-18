#include "AuthController.h"

#include "../service/AuthService.h"
#include "../repository/UserRepository.h"

#include <json/json.h>

void AuthController::registerUser(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto body = req->getJsonObject();

    if (!body)
    {
        Json::Value response;
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Invalid JSON";

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string name =
        (*body).get("name", "").asString();

    std::string email =
        (*body).get("email", "").asString();

    std::string password =
        (*body).get("password", "").asString();

    std::string role =
        (*body).get("role", "").asString();

    AuthService service;

    bool success = service.registerUser(
        name,
        email,
        password,
        role
    );

    Json::Value response;

    if (success)
    {
        response["success"] = true;
        response["data"]["message"] =
            "Registration successful";
        response["error"] = Json::nullValue;

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k201Created);
        callback(resp);
    }
    else
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] =
            "Registration failed";

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    }
}

void AuthController::loginUser(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto body = req->getJsonObject();

    Json::Value response;

    if (!body)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Invalid JSON";

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string email =
        (*body).get("email", "").asString();

    std::string password =
        (*body).get("password", "").asString();

    AuthService service;

    bool success =
        service.loginUser(email, password);

    if (!success)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] =
            "Invalid email or password";

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }
req->session()->insert("user_email", email);
    auto role =
        UserRepository().findRoleByEmail(email);

    response["success"] = true;
    response["data"]["message"] = "Login successful";
    response["data"]["email"] = email;
    response["data"]["role"] = role.value_or("");
    response["error"] = Json::nullValue;

    auto resp =
        drogon::HttpResponse::newHttpJsonResponse(response);

    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}