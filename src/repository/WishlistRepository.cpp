#include "WishlistRepository.h"
#include "Database.h"

bool WishlistRepository::addItem(long long userId, long long productId)
{
    auto dbClient = Database::getClient();

    try
    {
        dbClient->execSqlSync(
            "INSERT INTO wishlist_items (user_id, product_id) "
            "VALUES ($1, $2) "
            "ON CONFLICT (user_id, product_id) DO NOTHING",
            userId,
            productId
        );
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool WishlistRepository::removeItem(long long userId, long long productId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "DELETE FROM wishlist_items "
        "WHERE user_id = $1 AND product_id = $2",
        userId,
        productId
    );

    return result.affectedRows() > 0;
}

std::vector<WishlistItem> WishlistRepository::getItems(long long userId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT w.id, w.user_id, w.product_id, "
        "p.name AS product_name, p.price_cents, "
        "COALESCE(p.image_url, '') AS image_url, "
        "COALESCE(p.category, '') AS category "
        "FROM wishlist_items w "
        "JOIN products p ON w.product_id = p.id "
        "WHERE w.user_id = $1 "
        "ORDER BY w.created_at DESC",
        userId
    );

    std::vector<WishlistItem> items;

    for (const auto& row : result)
    {
        WishlistItem item;
        item.id = row["id"].as<long long>();
        item.userId = row["user_id"].as<long long>();
        item.productId = row["product_id"].as<long long>();
        item.productName = row["product_name"].as<std::string>();
        item.priceCents = row["price_cents"].as<long long>();
        item.imageUrl = row["image_url"].as<std::string>();
        item.category = row["category"].as<std::string>();
        items.push_back(item);
    }

    return items;
}

bool WishlistRepository::hasItem(long long userId, long long productId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT COUNT(*) AS count "
        "FROM wishlist_items "
        "WHERE user_id = $1 AND product_id = $2",
        userId,
        productId
    );

    return result[0]["count"].as<long long>() > 0;
}
