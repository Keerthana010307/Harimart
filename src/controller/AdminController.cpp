#include "AdminController.h"
#include "../util/AdminAuthUtil.h"
#include "../repository/Database.h"
#include "../service/ProductService.h"

void AdminController::getUsers(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    if (!isAdmin(req))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Admin access required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id, name, email, role "
        "FROM users "
        "ORDER BY id"
    );

    Json::Value users(Json::arrayValue);

    for (const auto& row : result)
    {
        Json::Value user;
        user["id"] = Json::Int64(row["id"].as<long long>());
        user["name"] = row["name"].as<std::string>();
        user["email"] = row["email"].as<std::string>();
        user["role"] = row["role"].as<std::string>();

        users.append(user);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["users"] = users;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}
void AdminController::getOrders(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    if (!isAdmin(req))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Admin access required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT o.id, o.buyer_id, u.name AS buyer_name, "
        "u.email AS buyer_email, o.status, "
        "o.total_amount_cents, o.created_at "
        "FROM orders o "
        "JOIN users u ON o.buyer_id = u.id "
        "ORDER BY o.id DESC"
    );

    Json::Value orders(Json::arrayValue);

    for (const auto& row : result)
    {
        Json::Value order;

        order["id"] = Json::Int64(row["id"].as<long long>());
        order["buyerId"] = Json::Int64(row["buyer_id"].as<long long>());
        order["buyerName"] = row["buyer_name"].as<std::string>();
        order["buyerEmail"] = row["buyer_email"].as<std::string>();
        order["status"] = row["status"].as<std::string>();
        order["totalAmountCents"] =
            Json::Int64(row["total_amount_cents"].as<long long>());
        order["createdAt"] = row["created_at"].as<std::string>();

        orders.append(order);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["orders"] = orders;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}
void AdminController::getProducts(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    if (!isAdmin(req))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Admin access required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT p.id, p.seller_id, "
        "u.name AS seller_name, u.email AS seller_email, "
        "p.name, p.description, p.price_cents, "
        "p.stock_qty, p.category, p.created_at "
        "FROM products p "
        "JOIN users u ON p.seller_id = u.id "
        "ORDER BY p.id DESC"
    );

    Json::Value products(Json::arrayValue);

    for (const auto& row : result)
    {
        Json::Value product;

        product["id"] =
            Json::Int64(row["id"].as<long long>());

        product["sellerId"] =
            Json::Int64(row["seller_id"].as<long long>());

        product["sellerName"] =
            row["seller_name"].as<std::string>();

        product["sellerEmail"] =
            row["seller_email"].as<std::string>();

        product["name"] =
            row["name"].as<std::string>();

        product["description"] =
            row["description"].as<std::string>();

        product["priceCents"] =
            Json::Int64(row["price_cents"].as<long long>());

        product["stock"] =
            row["stock_qty"].as<int>();

        product["category"] =
            row["category"].isNull()
                ? ""
                : row["category"].as<std::string>();

        product["createdAt"] =
            row["created_at"].as<std::string>();

        products.append(product);
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["products"] = products;
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}


void AdminController::deleteProduct(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
    long long productId)
{
    if (!isAdmin(req))
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] = "Admin access required";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k403Forbidden);
        callback(response);
        return;
    }

    ProductService service;

    bool success =
        service.adminDeleteProduct(productId);

    if (!success)
    {
        Json::Value responseJson;
        responseJson["success"] = false;
        responseJson["data"] = Json::nullValue;
        responseJson["error"]["message"] =
            "Product not found or could not be deleted";

        auto response =
            drogon::HttpResponse::newHttpJsonResponse(responseJson);

        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    Json::Value responseJson;
    responseJson["success"] = true;
    responseJson["data"]["message"] =
        "Product deleted successfully";
    responseJson["error"] = Json::nullValue;

    auto response =
        drogon::HttpResponse::newHttpJsonResponse(responseJson);

    response->setStatusCode(drogon::k200OK);
    callback(response);
}