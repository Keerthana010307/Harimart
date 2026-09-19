#include "ProductController.h"

#include "../service/ProductService.h"

#include <json/json.h>

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

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    try
    {
        long long sellerId =
            (*body).get("sellerId", 0).asInt64();

        std::string name =
            (*body).get("name", "").asString();

        std::string description =
            (*body).get("description", "").asString();

        long long priceCents =
            (*body).get("priceCents", 0).asInt64();

        int stock =
            (*body).get("stock", 0).asInt();

        ProductService service;

        bool success =
            service.createProduct(
                sellerId,
                name,
                description,
                priceCents,
                stock
            );

        if (!success)
        {
            response["success"] = false;
            response["data"] = Json::nullValue;
            response["error"]["message"] =
                "Product creation failed";

            auto resp =
                drogon::HttpResponse::newHttpJsonResponse(response);

            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }

        response["success"] = true;
        response["data"]["message"] =
            "Product created successfully";
        response["error"] = Json::nullValue;

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k201Created);
        callback(resp);
    }
    catch (const std::exception& e)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = e.what();

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

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

            item["id"] = product.id;
            item["sellerId"] = product.sellerId;
            item["name"] = product.name;
            item["description"] = product.description;
            item["priceCents"] =
                Json::Int64(product.priceCents);
            item["stock"] = product.stock;

            response["data"].append(item);
        }

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k200OK);
        callback(resp);
    }
    catch (const std::exception& e)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = e.what();

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

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

        auto product =
            service.getProductById(id);

        if (!product.has_value())
        {
            response["success"] = false;
            response["data"] = Json::nullValue;
            response["error"]["message"] =
                "Product not found";

            auto resp =
                drogon::HttpResponse::newHttpJsonResponse(response);

            resp->setStatusCode(drogon::k404NotFound);
            callback(resp);
            return;
        }

        response["success"] = true;
        response["error"] = Json::nullValue;

        response["data"]["id"] =
            product->id;

        response["data"]["sellerId"] =
            Json::Int64(product->sellerId);

        response["data"]["name"] =
            product->name;

        response["data"]["description"] =
            product->description;

        response["data"]["priceCents"] =
            Json::Int64(product->priceCents);

        response["data"]["stock"] =
            product->stock;

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k200OK);
        callback(resp);
    }
    catch (const std::exception& e)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] = e.what();

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

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

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    long long sellerId =
        (*body).get("sellerId", 0).asInt64();

    std::string name =
        (*body).get("name", "").asString();

    std::string description =
        (*body).get("description", "").asString();

    long long priceCents =
        (*body).get("priceCents", 0).asInt64();

    int stock =
        (*body).get("stock", 0).asInt();

    ProductService service;

    bool success =
        service.updateProduct(
            id,
            sellerId,
            name,
            description,
            priceCents,
            stock
        );

    if (!success)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] =
            "Product update failed";

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    response["success"] = true;
    response["data"]["message"] =
        "Product updated successfully";
    response["error"] = Json::nullValue;

    auto resp =
        drogon::HttpResponse::newHttpJsonResponse(response);

    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}

void ProductController::deleteProduct(
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

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    int sellerId =
        (*body).get("sellerId", 0).asInt();

    ProductService service;

    bool success =
        service.deleteProduct(
            id,
            sellerId
        );

    if (!success)
    {
        response["success"] = false;
        response["data"] = Json::nullValue;
        response["error"]["message"] =
            "Product deletion failed";

        auto resp =
            drogon::HttpResponse::newHttpJsonResponse(response);

        resp->setStatusCode(drogon::k404NotFound);
        callback(resp);
        return;
    }

    response["success"] = true;
    response["data"]["message"] =
        "Product deleted successfully";
    response["error"] = Json::nullValue;

    auto resp =
        drogon::HttpResponse::newHttpJsonResponse(response);

    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}