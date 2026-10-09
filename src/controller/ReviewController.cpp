#include "ReviewController.h"
#include "../service/ReviewService.h"
#include "../repository/UserRepository.h"

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
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!json->isMember("productId") || !json->isMember("rating"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "productId and rating are required";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!req->session()->find("user_email"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required to submit a review";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    std::string email = req->session()->get<std::string>("user_email");
    UserRepository userRepository;
    auto userIdOptional = userRepository.findUserIdByEmail(email);

    if (!userIdOptional.has_value())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "User not found";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    long long userId = userIdOptional.value();
    long long productId = (*json)["productId"].asInt64();
    int rating = (*json)["rating"].asInt();
    std::string comment = json->isMember("comment")
        ? (*json)["comment"].asString() : "";

    ReviewService service;
    auto result = service.addReview(productId, userId, rating, comment);

    if (!result.success)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = result.errorMessage;
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    responseJson["success"] = true;
    responseJson["data"]["message"] = "Review added successfully";
    responseJson["error"] = Json::nullValue;
    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
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

    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k200OK);
    callback(response);
}