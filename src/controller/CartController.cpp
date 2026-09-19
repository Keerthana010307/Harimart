#include "CartController.h"
#include "../service/CartService.h"

#include <nlohmann/json.hpp>
#include <json/json.h>
#include <string>
#include <functional>

using json = nlohmann::json;

void CartController::addToCart(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto jsonBody = req->getJsonObject();

    if (!jsonBody)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "Invalid JSON";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!jsonBody->isMember("userId") ||
        !jsonBody->isMember("productId") ||
        !jsonBody->isMember("quantity"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "userId, productId and quantity are required";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long userId = (*jsonBody)["userId"].asInt64();
    long long productId = (*jsonBody)["productId"].asInt64();
    int quantity = (*jsonBody)["quantity"].asInt();

    CartService service;

    if (!service.addToCart(userId, productId, quantity))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "Unable to add product to cart";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["error"] = Json::nullValue;
    responseJson["data"]["message"] = "Product added to cart successfully";

    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k201Created);
    callback(response);
}

void CartController::getCart(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto userIdParam = req->getParameter("userId");

    if (userIdParam.empty())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "userId is required";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long userId = std::stoll(userIdParam);

    CartService service;
    auto items = service.getCart(userId);

    Json::Value itemsJson(Json::arrayValue);

    for (const auto& item : items)
    {
        Json::Value itemJson;
        itemJson["id"] = Json::Int64(item.id);
        itemJson["userId"] = Json::Int64(item.userId);
        itemJson["productId"] = Json::Int64(item.productId);
        itemJson["quantity"] = item.quantity;

        itemsJson.append(itemJson);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["error"] = Json::nullValue;
    responseJson["data"]["items"] = itemsJson;

    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k200OK);
    callback(response);
}

void CartController::updateCart(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long id)
{
    auto jsonBody = req->getJsonObject();

    if (!jsonBody)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "Invalid JSON";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    if (!jsonBody->isMember("userId") ||
        !jsonBody->isMember("quantity"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "userId and quantity are required";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long productId = id;
    long long userId = (*jsonBody)["userId"].asInt64();
    int quantity = (*jsonBody)["quantity"].asInt();

    CartService service;

    if (!service.updateCart(userId, productId, quantity))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "Unable to update cart";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["error"] = Json::nullValue;
    responseJson["data"]["message"] = "Cart updated successfully";

    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k200OK);
    callback(response);
}

void CartController::removeFromCart(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long id)
{
    auto jsonBody = req->getJsonObject();

    if (!jsonBody || !jsonBody->isMember("userId"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "userId is required";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long productId = id;
    long long userId = (*jsonBody)["userId"].asInt64();

    CartService service;

    if (!service.removeFromCart(userId, productId))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["error"] = "Unable to remove product from cart";
        responseJson["data"] = Json::nullValue;

        auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["error"] = Json::nullValue;
    responseJson["data"]["message"] = "Product removed from cart successfully";

    auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    response->setStatusCode(drogon::k200OK);
    callback(response);
}