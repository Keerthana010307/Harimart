#include "CartRepository.h"
#include "../repository/Database.h"

bool CartRepository::addOrUpdateItem(
    long long userId,
    long long productId,
    int quantity)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "INSERT INTO cart_items (user_id, product_id, quantity) "
        "VALUES ($1, $2, $3) "
        "ON CONFLICT (user_id, product_id) "
        "DO UPDATE SET quantity = cart_items.quantity + EXCLUDED.quantity",
        userId,
        productId,
        quantity
    );

    return result.affectedRows() > 0;
}

bool CartRepository::updateQuantity(
    long long userId,
    long long productId,
    int quantity)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "UPDATE cart_items "
        "SET quantity = $1 "
        "WHERE user_id = $2 AND product_id = $3",
        quantity,
        userId,
        productId
    );

    return result.affectedRows() > 0;
}

bool CartRepository::removeItem(
    long long userId,
    long long productId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "DELETE FROM cart_items "
        "WHERE user_id = $1 AND product_id = $2",
        userId,
        productId
    );

    return result.affectedRows() > 0;
}

std::vector<CartItem> CartRepository::getCartItems(
    long long userId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT c.id, c.user_id, c.product_id, c.quantity, p.price_cents "
"FROM cart_items c "
"JOIN products p ON c.product_id = p.id "
        "WHERE user_id = $1 "
        "ORDER BY id",
        userId
    );

    std::vector<CartItem> items;

    for (const auto& row : result)
    {
        CartItem item;

        item.id = row["id"].as<long long>();
        item.userId = row["user_id"].as<long long>();
        item.productId = row["product_id"].as<long long>();
        item.quantity = row["quantity"].as<int>();
        item.priceCents = row["price_cents"].as<long long>();

        items.push_back(item);
    }

    return items;
}
bool CartRepository::clearCart(long long userId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "DELETE FROM cart_items "
        "WHERE user_id = $1",
        userId
    );

    return result.affectedRows() > 0;
}