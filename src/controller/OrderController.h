#pragma once

#include <drogon/HttpController.h>
#include <functional>

class OrderController
    : public drogon::HttpController<OrderController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        OrderController::createOrder,
        "/api/v1/orders",
        drogon::Post
    );

    ADD_METHOD_TO(
        OrderController::getOrders,
        "/api/v1/orders",
        drogon::Get
    );

    ADD_METHOD_TO(
    OrderController::confirmOrder,
    "/api/v1/orders/{1}/confirm",
    drogon::Post
);

    METHOD_LIST_END

    void createOrder(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getOrders(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
    void confirmOrder(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long orderId
);

};