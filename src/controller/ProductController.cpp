#include "ProductController.h"
#include "../service/ProductService.h"
#include "../repository/UserRepository.h"

#include <json/json.h>
#include <drogon/orm/Exception.h>

void ProductController::createProduct(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto body = req->getJsonObject();
    Json::Value response;

    if (!body)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Invalid JSON";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    // Session-based auth: seller must be logged in
    if (!req->session()->find("user_email"))
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Login required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    std::string email = req->session()->get<std::string>("user_email");
    UserRepository userRepository;

    auto roleOpt = userRepository.findRoleByEmail(email);
    if (!roleOpt.has_value() || roleOpt.value() != "SELLER")
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Seller access required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k403Forbidden);
        callback(resp);
        return;
    }

    auto userIdOpt = userRepository.findUserIdByEmail(email);
    if (!userIdOpt.has_value())
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "User not found";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    long long sellerId = userIdOpt.value();

    try
    {
        std::string name = (*body).get("name", "").asString();
        std::string description = (*body).get("description", "").asString();
        long long priceCents = (*body).get("priceCents", 0).asInt64();
        int stock = (*body).get("stock", 0).asInt();
        std::string category = (*body).get("category", "").asString();
        std::string imageUrl = (*body).get("imageUrl", "").asString();

        ProductService service;

        bool success = service.createProduct(
            sellerId, name, description,
            priceCents, stock, category, imageUrl
        );

        if (!success)
        {
            response["success"] = false;
            response["data"] = Json::nullValue;
            response["error"]["message"] = "Product creation failed. Check required fields.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }

        response["success"] = true;
        response["data"]["message"] = "Product created successfully";
        response["error"] = Json::nullValue;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k201Created);
        callback(resp);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "[ProductController] createProduct DB error: "
                  << e.base().what() << std::endl;
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = e.base().what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ProductController] createProduct error: "
                  << e.what() << std::endl;
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = e.what();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void ProductController::getAllProducts(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    Json::Value response;

    try
    {
        ProductService service;
        auto products = service.getAllProducts();

        response["success"] = true;
        response["error"] = Json::nullValue;
        response["data"] = Json::arrayValue;

        for (const auto& product : products)
        {
            Json::Value item;
            item["id"] = Json::Int64(product.id);
            item["sellerId"] = Json::Int64(product.sellerId);
            item["name"] = product.name;
            item["description"] = product.description;
            item["priceCents"] = Json::Int64(product.priceCents);
            item["stock"] = product.stock;
            item["category"] = product.category;
            item["imageUrl"] = product.imageUrl;
            response["data"].append(item);
        }

        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k200OK);
        callback(resp);
    }
    catch (const std::exception& e)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Internal server error";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void ProductController::getProductById(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long id)
{
    Json::Value response;

    try
    {
        ProductService service;
        auto product = service.getProductById(id);

        if (!product.has_value())
        {
            response["success"] = false;
            response["data"] = Json::nullValue;
            response["error"]["message"] = "Product not found";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }

        response["success"] = true;
        response["error"] = Json::nullValue;
        response["data"]["id"] = Json::Int64(product->id);
        response["data"]["sellerId"] = Json::Int64(product->sellerId);
        response["data"]["name"] = product->name;
        response["data"]["description"] = product->description;
        response["data"]["priceCents"] = Json::Int64(product->priceCents);
        response["data"]["stock"] = product->stock;
        response["data"]["category"] = product->category;
        response["data"]["imageUrl"] = product->imageUrl;

        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k200OK);
        callback(resp);
    }
    catch (const std::exception& e)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Internal server error";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    }
}

void ProductController::updateProduct(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long id)
{
    auto body = req->getJsonObject();
    Json::Value response;

    if (!body)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Invalid JSON";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    if (!req->session()->find("user_email"))
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Login required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    std::string email = req->session()->get<std::string>("user_email");
    UserRepository userRepository;

    auto userIdOpt = userRepository.findUserIdByEmail(email);
    if (!userIdOpt.has_value())
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "User not found";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    long long sellerId = userIdOpt.value();

    std::string name = (*body).get("name", "").asString();
    std::string description = (*body).get("description", "").asString();
    long long priceCents = (*body).get("priceCents", 0).asInt64();
    int stock = (*body).get("stock", 0).asInt();
    std::string category = (*body).get("category", "").asString();
    std::string imageUrl = (*body).get("imageUrl", "").asString();

    ProductService service;

    bool success = service.updateProduct(
        id, sellerId, name, description,
        priceCents, stock, category, imageUrl
    );

    if (!success)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Product update failed or unauthorized";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    response["success"] = true;
    response["data"]["message"] = "Product updated successfully";
    response["error"] = Json::nullValue;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

void ProductController::deleteProduct(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long id)
{
    Json::Value response;

    if (!req->session()->find("user_email"))
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Login required";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    std::string email = req->session()->get<std::string>("user_email");
    UserRepository userRepository;

    auto userIdOpt = userRepository.findUserIdByEmail(email);
    if (!userIdOpt.has_value())
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "User not found";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
        return;
    }

    long long sellerId = userIdOpt.value();

    ProductService service;
    bool success = service.deleteProduct(id, sellerId);

    if (!success)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = "Product deletion failed or unauthorized";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    response["success"] = true;
    response["data"]["message"] = "Product deleted successfully";
    response["error"] = Json::nullValue;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}