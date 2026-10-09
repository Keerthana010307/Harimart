#include "ProductRepository.h"
#include "Database.h"
#include <drogon/orm/Exception.h>
#include <iostream>

bool ProductRepository::createProduct(
    long long sellerId,
    const std::string& name,
    const std::string& description,
    long long priceCents,
    int stock,
    const std::string& category,
    const std::string& imageUrl)
{
    auto dbClient = Database::getClient();

    try
    {
        dbClient->execSqlSync(
            "INSERT INTO products "
            "(seller_id, name, description, price_cents, stock_qty, category, image_url) "
            "VALUES ($1, $2, $3, $4, $5, $6, $7)",
            sellerId,
            name,
            description,
            priceCents,
            stock,
            category,
            imageUrl
        );
        return true;
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "[ProductRepository] createProduct DB error: "
                  << e.base().what() << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[ProductRepository] createProduct error: "
                  << e.what() << std::endl;
        return false;
    }
}

std::vector<Product> ProductRepository::getAllProducts()
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id, seller_id, name, description, "
        "price_cents, stock_qty, "
        "COALESCE(category, '') AS category, "
        "COALESCE(image_url, '') AS image_url "
        "FROM products "
        "ORDER BY id DESC"
    );

    std::vector<Product> products;

    for (const auto& row : result)
    {
        Product product;
        product.id = row["id"].as<long long>();
        product.sellerId = row["seller_id"].as<long long>();
        product.name = row["name"].as<std::string>();
        product.description = row["description"].isNull()
            ? "" : row["description"].as<std::string>();
        product.priceCents = row["price_cents"].as<long long>();
        product.stock = row["stock_qty"].as<int>();
        product.category = row["category"].as<std::string>();
        product.imageUrl = row["image_url"].as<std::string>();
        products.push_back(product);
    }

    return products;
}

std::optional<Product> ProductRepository::getProductById(long long id)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id, seller_id, name, description, "
        "price_cents, stock_qty, "
        "COALESCE(category, '') AS category, "
        "COALESCE(image_url, '') AS image_url "
        "FROM products "
        "WHERE id = $1",
        id
    );

    if (result.empty())
    {
        return std::nullopt;
    }

    Product product;
    product.id = result[0]["id"].as<long long>();
    product.sellerId = result[0]["seller_id"].as<long long>();
    product.name = result[0]["name"].as<std::string>();
    product.description = result[0]["description"].isNull()
        ? "" : result[0]["description"].as<std::string>();
    product.priceCents = result[0]["price_cents"].as<long long>();
    product.stock = result[0]["stock_qty"].as<int>();
    product.category = result[0]["category"].as<std::string>();
    product.imageUrl = result[0]["image_url"].as<std::string>();

    return product;
}

bool ProductRepository::updateProduct(
    long long id,
    long long sellerId,
    const std::string& name,
    const std::string& description,
    long long priceCents,
    int stock,
    const std::string& category,
    const std::string& imageUrl)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "UPDATE products "
        "SET name = $1, "
        "description = $2, "
        "price_cents = $3, "
        "stock_qty = $4, "
        "category = $5, "
        "image_url = $6 "
        "WHERE id = $7 AND seller_id = $8",
        name,
        description,
        priceCents,
        stock,
        category,
        imageUrl,
        id,
        sellerId
    );

    return result.affectedRows() > 0;
}

bool ProductRepository::deleteProduct(
    long long id,
    long long sellerId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "DELETE FROM products "
        "WHERE id = $1 AND seller_id = $2",
        id,
        sellerId
    );

    return result.affectedRows() > 0;
}

bool ProductRepository::adminDeleteProduct(long long id)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "DELETE FROM products "
        "WHERE id = $1",
        id
    );

    return result.affectedRows() > 0;
}
