#pragma once

#include <drogon/HttpController.h>

class WishlistController
    : public drogon::HttpController<WishlistController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        WishlistController::addToWishlist,
        "/api/v1/wishlist",
        drogon::Post
    );

    ADD_METHOD_TO(
        WishlistController::getWishlist,
        "/api/v1/wishlist",
        drogon::Get
    );

    ADD_METHOD_TO(
        WishlistController::removeFromWishlist,
        "/api/v1/wishlist/{1}",
        drogon::Delete
    );

    METHOD_LIST_END

    void addToWishlist(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getWishlist(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void removeFromWishlist(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long productId
    );
};
