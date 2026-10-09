#include "WishlistController.h"
#include "../service/WishlistService.h"
#include "../repository/UserRepository.h"

#include <json/json.h>

void WishlistController::addToWishlist(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = req->getJsonObject();
    Json::Value responseJson;

    if (!jsonBody || !jsonBody->isMember("productId"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "productId is required";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!req->session()->find("user_email"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    std::string email = req->session()->get<std::string>("user_email");
    UserRepository userRepository;
    auto userIdOpt = userRepository.findUserIdByEmail(email);

    if (!userIdOpt.has_value())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "User not found";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    long long userId = userIdOpt.value();
    long long productId = (*jsonBody)["productId"].asInt64();

    WishlistService service;
    bool success = service.addToWishlist(userId, productId);

    if (!success)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Unable to add to wishlist";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    responseJson["success"] = true;
    responseJson["data"]["message"] = "Added to wishlist";
    responseJson["error"] = Json::nullValue;
    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k201Created);
    callback(response);
}

void WishlistController::getWishlist(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    Json::Value responseJson;

    if (!req->session()->find("user_email"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    std::string email = req->session()->get<std::string>("user_email");
    UserRepository userRepository;
    auto userIdOpt = userRepository.findUserIdByEmail(email);

    if (!userIdOpt.has_value())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "User not found";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    long long userId = userIdOpt.value();
    WishlistService service;
    auto items = service.getWishlist(userId);

    Json::Value itemsJson(Json::arrayValue);

    for (const auto& item : items)
    {
        Json::Value itemJson;
        itemJson["id"] = Json::Int64(item.id);
        itemJson["productId"] = Json::Int64(item.productId);
        itemJson["productName"] = item.productName;
        itemJson["priceCents"] = Json::Int64(item.priceCents);
        itemJson["imageUrl"] = item.imageUrl;
        itemJson["category"] = item.category;
        itemsJson.append(itemJson);
    }

    responseJson["success"] = true;
    responseJson["data"]["items"] = itemsJson;
    responseJson["error"] = Json::nullValue;
    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k200OK);
    callback(response);
}

void WishlistController::removeFromWishlist(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long productId)
{
    Json::Value responseJson;

    if (!req->session()->find("user_email"))
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    std::string email = req->session()->get<std::string>("user_email");
    UserRepository userRepository;
    auto userIdOpt = userRepository.findUserIdByEmail(email);

    if (!userIdOpt.has_value())
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "User not found";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    long long userId = userIdOpt.value();
    WishlistService service;
    bool success = service.removeFromWishlist(userId, productId);

    if (!success)
    {
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Item not found in wishlist";
        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k404NotFound);
        callback(response);
        return;
    }

    responseJson["success"] = true;
    responseJson["data"]["message"] = "Removed from wishlist";
    responseJson["error"] = Json::nullValue;
    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k200OK);
    callback(response);
}
