#pragma once

#include <drogon/HttpController.h>

class AdminController
    : public drogon::HttpController<AdminController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        AdminController::getUsers,
        "/api/v1/admin/users",
        drogon::Get
    );

    METHOD_LIST_END

    void getUsers(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
};
