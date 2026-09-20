#include "ProductRepository.h"

#include "Database.h"

bool ProductRepository::createProduct(
    long long sellerId,
    const std::string& name,
    const std::string& description,
    long long priceCents,
    int stock)
{
    auto dbClient = Database::getClient();

    dbClient->execSqlSync(
        "INSERT INTO products "
        "(seller_id, name, description, price_cents, stock_qty) "
        "VALUES ($1, $2, $3, $4, $5)",
        sellerId,
        name,
        description,
        priceCents,
        stock
    );

    return true;
}

std::vector<Product> ProductRepository::getAllProducts()
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id, seller_id, name, description, "
        "price_cents, stock_qty "
        "FROM products "
        "ORDER BY id DESC"
    );

    std::vector<Product> products;

    for (const auto& row : result)
    {
        Product product;

        product.id =
            row["id"].as<int>();

        product.sellerId =
            row["seller_id"].as<int>();

        product.name =
            row["name"].as<std::string>();

        product.description =
            row["description"].as<std::string>();

        product.priceCents =
            row["price_cents"].as<long long>();

        product.stock =
            row["stock_qty"].as<int>();

        products.push_back(product);
    }

    return products;
}

std::optional<Product> ProductRepository::getProductById(long long  id)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id, seller_id, name, description, "
        "price_cents, stock_qty "
        "FROM products "
        "WHERE id = $1",
        id
    );

    if (result.empty())
    {
        return std::nullopt;
    }

    Product product;

    product.id =
        result[0]["id"].as<int>();

    product.sellerId =
        result[0]["seller_id"].as<int>();

    product.name =
        result[0]["name"].as<std::string>();

    product.description =
        result[0]["description"].as<std::string>();

    product.priceCents =
        result[0]["price_cents"].as<long long>();

    product.stock =
        result[0]["stock_qty"].as<int>();

    return product;
}

bool ProductRepository::updateProduct(
    long long  id,
    long long sellerId,
    const std::string& name,
    const std::string& description,
    long long priceCents,
    int stock)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "UPDATE products "
        "SET name = $1, "
        "description = $2, "
        "price_cents = $3, "
        "stock_qty = $4 "
        "WHERE id = $5 AND seller_id = $6",
        name,
        description,
        priceCents,
        stock,
        id,
        sellerId
    );

    return result.affectedRows() > 0;
}

bool ProductRepository::deleteProduct(
    long long  id,
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

bool ProductRepository::adminDeleteProduct(
    long long id)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "DELETE FROM products "
        "WHERE id = $1",
        id
    );

    return result.affectedRows() > 0;
}
