#pragma once

#include <drogon/HttpController.h>

class CartController
    : public drogon::HttpController<CartController>
{
public:

    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        CartController::addToCart,
        "/api/v1/cart",
        drogon::Post
    );

    ADD_METHOD_TO(
        CartController::getCart,
        "/api/v1/cart",
        drogon::Get
    );

    ADD_METHOD_TO(
        CartController::updateCart,
        "/api/v1/cart/{1}",
        drogon::Put
    );

    ADD_METHOD_TO(
        CartController::removeFromCart,
        "/api/v1/cart/{1}",
        drogon::Delete
    );

    METHOD_LIST_END

    void addToCart(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getCart(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void updateCart(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long id
    );

    void removeFromCart(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long id
    );
};