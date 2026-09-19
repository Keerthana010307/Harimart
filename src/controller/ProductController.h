#pragma once

#include <drogon/HttpController.h>

class ProductController
    : public drogon::HttpController<ProductController>
{
public:

    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        ProductController::createProduct,
        "/api/v1/products",
        drogon::Post
    );

    ADD_METHOD_TO(
        ProductController::getAllProducts,
        "/api/v1/products",
        drogon::Get
    );

    ADD_METHOD_TO(
        ProductController::getProductById,
        "/api/v1/products/{1}",
        drogon::Get
    );

    ADD_METHOD_TO(
        ProductController::updateProduct,
        "/api/v1/products/{1}",
        drogon::Put
    );

    ADD_METHOD_TO(
        ProductController::deleteProduct,
        "/api/v1/products/{1}",
        drogon::Delete
    );

    METHOD_LIST_END

    void createProduct(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getAllProducts(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void getProductById(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long id
    );

    void updateProduct(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long id
    );

    void deleteProduct(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        long long id
    );
};