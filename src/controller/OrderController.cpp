#include "OrderController.h"
#include "../service/OrderService.h"
#include "../repository/CartRepository.h"

void OrderController::createOrder(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto userIdParam = req->getParameter("userId");

    if (userIdParam.empty())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "userId is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long userId = std::stoll(userIdParam);

    OrderService service;

    long long orderId =
        service.createOrder(userId);

    if (orderId <= 0)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "Cart is empty";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["orderId"] =
        Json::Int64(orderId);
    responseJson["data"]["message"] =
        "Order created successfully";
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k201Created);
    callback(response);
}

void OrderController::getOrders(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto userIdParam = req->getParameter("userId");

    if (userIdParam.empty())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "userId is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long userId = std::stoll(userIdParam);

    OrderService service;

    auto orders =
        service.getOrdersByBuyer(userId);

    Json::Value ordersJson(Json::arrayValue);

    for (const auto& order : orders)
    {
        Json::Value orderJson;

        orderJson["id"] =
            Json::Int64(order.id);

        orderJson["buyerId"] =
            Json::Int64(order.buyerId);

        orderJson["status"] =
            order.status;

        orderJson["totalAmountCents"] =
            Json::Int64(order.totalAmountCents);

        ordersJson.append(orderJson);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["orders"] = ordersJson;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}
void OrderController::confirmOrder(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long orderId)
{
    OrderService service;

    bool success = service.confirmOrder(orderId);
    if (success)
{
    auto userIdParam = req->getParameter("userId");

    if (!userIdParam.empty())
    {
        long long userId = std::stoll(userIdParam);

        CartRepository cartRepository;
        cartRepository.clearCart(userId);
    }
}

    Json::Value responseJson;

    if (success)
    {
        responseJson["success"] = true;
        responseJson["data"]["orderId"] =
            Json::Int64(orderId);
        responseJson["data"]["paymentStatus"] =
            "SUCCESS";
        responseJson["data"]["message"] =
            "Mock payment successful";
        responseJson["error"] = Json::nullValue;

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k200OK);
        callback(response);
        return;
    }

    responseJson["success"] = false;
    responseJson["data"] = Json::nullValue;
    responseJson["error"]["message"] =
        "Payment failed";

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k400BadRequest);
    callback(response);
}
void OrderController::getSellerOrders(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto sellerIdParam = req->getParameter("sellerId");

    if (sellerIdParam.empty())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "sellerId is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    long long sellerId = std::stoll(sellerIdParam);

    OrderService service;

    auto orders =
        service.getOrdersBySeller(sellerId);

    Json::Value ordersJson(Json::arrayValue);

    for (const auto& order : orders)
    {
        Json::Value orderJson;

        orderJson["id"] =
            Json::Int64(order.id);

        orderJson["buyerId"] =
            Json::Int64(order.buyerId);

        orderJson["status"] =
            order.status;

        orderJson["totalAmountCents"] =
            Json::Int64(order.totalAmountCents);

        ordersJson.append(orderJson);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["orders"] = ordersJson;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}
void OrderController::updateOrderStatus(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long orderId)
{
    auto status = req->getParameter("status");

    if (status.empty())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "status is required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    OrderService service;

    bool success =
        service.updateOrderStatus(orderId, status);

    Json::Value responseJson;

    if (success)
    {
        responseJson["success"] = true;
        responseJson["data"]["orderId"] =
            Json::Int64(orderId);
        responseJson["data"]["status"] = status;
        responseJson["error"] = Json::nullValue;

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k200OK);
        callback(response);
        return;
    }

    responseJson["success"] = false;
    responseJson["data"] = Json::nullValue;
    responseJson["error"]["message"] =
        "Invalid order status or order update failed";

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k400BadRequest);
    callback(response);
}
