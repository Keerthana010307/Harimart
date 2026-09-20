#pragma once

#include <drogon/HttpController.h>

class ReviewController
    : public drogon::HttpController<ReviewController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ReviewController::addReview,
        "/api/v1/reviews",
        drogon::Post
    );

    ADD_METHOD_TO(
        ReviewController::getProductReviews,
        "/api/v1/reviews/{1}",
        drogon::Get
    );

    METHOD_LIST_END

    void addReview(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getProductReviews(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long productId
    );
};