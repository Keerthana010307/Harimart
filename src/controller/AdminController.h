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

    ADD_METHOD_TO(
        AdminController::getOrders,
        "/api/v1/admin/orders",
        drogon::Get
    );

    ADD_METHOD_TO(
        AdminController::getProducts,
        "/api/v1/admin/products",
        drogon::Get
    );

    ADD_METHOD_TO(
        AdminController::deleteProduct,
        "/api/v1/admin/products/{1}",
        drogon::Delete
    );

    METHOD_LIST_END

    void getUsers(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getOrders(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getProducts(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void deleteProduct(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long productId
    );
};