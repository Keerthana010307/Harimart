#include "ReviewController.h"
#include "../service/ReviewService.h"

void ReviewController::addReview(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    Json::Value responseJson;

    if (!json)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Invalid JSON";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!json->isMember("productId") ||
        !json->isMember("userId") ||
        !json->isMember("rating"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "productId, userId and rating are required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long productId = (*json)["productId"].asInt64();
    long long userId = (*json)["userId"].asInt64();
    int rating = (*json)["rating"].asInt();

    std::string comment = "";

    if (json->isMember("comment"))
    {
        comment = (*json)["comment"].asString();
    }

    ReviewService service;

    bool success = service.addReview(
        productId,
        userId,
        rating,
        comment
    );

    if (!success)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "Invalid rating or review already exists";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    responseJson["success"] = true;
    responseJson["data"]["message"] = "Review added successfully";
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k201Created);
    callback(response);
}

void ReviewController::getProductReviews(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long productId)
{
    ReviewService service;

    auto reviews = service.getProductReviews(productId);

    Json::Value reviewList(Json::arrayValue);

    for (const auto& review : reviews)
    {
        Json::Value item;

        item["id"] = Json::Int64(review.id);
        item["productId"] = Json::Int64(review.productId);
        item["userId"] = Json::Int64(review.userId);
        item["rating"] = review.rating;
        item["comment"] = review.comment;

        reviewList.append(item);
    }

    Json::Value responseJson;

    responseJson["success"] = true;
    responseJson["data"]["reviews"] = reviewList;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}