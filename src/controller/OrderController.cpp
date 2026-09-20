#include "OrderController.h"
#include "../service/OrderService.h"
#include "../repository/UserRepository.h"
#include "../repository/CartRepository.h"

void OrderController::createOrder(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{

if (!req->session()->find("user_email"))
{
    Json::Value responseJson;
    responseJson["success"] = false;
    responseJson["data"] = Json::nullValue;
    responseJson["error"]["message"] = "Login required";

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k401Unauthorized);
    callback(response);
    return;
}

std::string email =
    req->session()->get<std::string>("user_email");

UserRepository userRepository;

auto userIdOptional =
    userRepository.findUserIdByEmail(email);

if (!userIdOptional.has_value())
{
    Json::Value responseJson;
    responseJson["success"] = false;
    responseJson["data"] = Json::nullValue;
    responseJson["error"]["message"] = "User not found";

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k401Unauthorized);
    callback(response);
    return;
}

long long userId = userIdOptional.value();
    

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
    if (!req->session()->find("user_email"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    std::string email =
        req->session()->get<std::string>("user_email");

    UserRepository userRepository;

    auto userIdOptional =
        userRepository.findUserIdByEmail(email);

    if (!userIdOptional.has_value())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "User not found";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    long long userId = userIdOptional.value();

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
    if (!req->session()->find("user_email"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    std::string email =
        req->session()->get<std::string>("user_email");

    UserRepository userRepository;

    auto userIdOptional =
        userRepository.findUserIdByEmail(email);

    if (!userIdOptional.has_value())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "User not found";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    long long userId = userIdOptional.value();

    OrderService service;
    if (!service.isOrderOwnedByBuyer(orderId, userId))
{
    Json::Value responseJson;
    responseJson["success"] = false;
    responseJson["data"] = Json::nullValue;
    responseJson["error"]["message"] =
        "You are not allowed to confirm this order";

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k403Forbidden);
    callback(response);
    return;
}

    bool success = service.confirmOrder(orderId);

    if (success)
    {
        CartRepository cartRepository;
        cartRepository.clearCart(userId);
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
    if (!req->session()->find("user_email"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

    std::string email =
        req->session()->get<std::string>("user_email");

    UserRepository userRepository;

    auto userIdOptional =
        userRepository.findUserIdByEmail(email);

    if (!userIdOptional.has_value())
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "User not found";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }
    UserRepository roleRepository;

auto roleOptional =
    roleRepository.findRoleByEmail(email);

if (!roleOptional.has_value() ||
    (roleOptional.value() != "SELLER" &&
     roleOptional.value() != "ADMIN"))
{
    Json::Value responseJson;
    responseJson["success"] = false;
    responseJson["data"] = Json::nullValue;
    responseJson["error"]["message"] =
        "Seller or Admin access required";

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k403Forbidden);
    callback(response);
    return;
}

    long long sellerId = userIdOptional.value();

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
    if (!req->session()->find("user_email"))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Login required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k401Unauthorized);
        callback(response);
        return;
    }

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