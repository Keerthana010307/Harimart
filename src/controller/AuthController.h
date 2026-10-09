#pragma once

#include <drogon/HttpController.h>

class AuthController : public drogon::HttpController<AuthController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        AuthController::registerUser,
        "/api/v1/auth/register",
        drogon::Post
    );

    ADD_METHOD_TO(
        AuthController::loginUser,
        "/api/v1/auth/login",
        drogon::Post
    );

    ADD_METHOD_TO(
        AuthController::logoutUser,
        "/api/v1/auth/logout",
        drogon::Post
    );

    ADD_METHOD_TO(
        AuthController::getSession,
        "/api/v1/auth/session",
        drogon::Get
    );

    METHOD_LIST_END

    void registerUser(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void loginUser(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void logoutUser(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getSession(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
};