#include "OrderRepository.h"
#include "Database.h"

long long OrderRepository::createOrder(
    long long buyerId,
    long long totalAmountCents)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "INSERT INTO orders (buyer_id, total_amount_cents) "
        "VALUES ($1, $2) "
        "RETURNING id",
        buyerId,
        totalAmountCents
    );

    return result[0]["id"].as<long long>();
}

bool OrderRepository::addOrderItem(
    long long orderId,
    long long productId,
    int quantity,
    long long unitPriceCents)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "INSERT INTO order_items "
        "(order_id, product_id, quantity, unit_price_cents) "
        "VALUES ($1, $2, $3, $4)",
        orderId,
        productId,
        quantity,
        unitPriceCents
    );

    return result.affectedRows() > 0;
}

std::vector<Order> OrderRepository::getOrdersByBuyer(
    long long buyerId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id, buyer_id, status, total_amount_cents "
        "FROM orders "
        "WHERE buyer_id = $1 "
        "ORDER BY id DESC",
        buyerId
    );

    std::vector<Order> orders;

    for (const auto& row : result)
    {
        Order order;

        order.id = row["id"].as<long long>();
        order.buyerId = row["buyer_id"].as<long long>();
        order.status = row["status"].as<std::string>();
        order.totalAmountCents =
            row["total_amount_cents"].as<long long>();

        orders.push_back(order);
    }

    return orders;
}
bool OrderRepository::updateOrderStatus(
    long long orderId,
    const std::string& status)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "UPDATE orders "
        "SET status = $1 "
        "WHERE id = $2",
        status,
        orderId
    );

    return result.affectedRows() > 0;
}
std::vector<Order> OrderRepository::getOrdersBySeller(
    long long sellerId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT DISTINCT o.id, o.buyer_id, o.status, "
        "o.total_amount_cents "
        "FROM orders o "
        "JOIN order_items oi ON o.id = oi.order_id "
        "JOIN products p ON oi.product_id = p.id "
        "WHERE p.seller_id = $1 "
        "ORDER BY o.id DESC",
        sellerId
    );

    std::vector<Order> orders;

    for (const auto& row : result)
    {
        Order order;

        order.id = row["id"].as<long long>();
        order.buyerId = row["buyer_id"].as<long long>();
        order.status = row["status"].as<std::string>();
        order.totalAmountCents =
            row["total_amount_cents"].as<long long>();

        orders.push_back(order);
    }

    return orders;
}
bool OrderRepository::hasCompletedPurchase(
    long long buyerId,
    long long productId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT COUNT(*) AS count "
        "FROM orders o "
        "JOIN order_items oi ON o.id = oi.order_id "
        "WHERE o.buyer_id = $1 "
        "AND oi.product_id = $2 "
        "AND o.status = 'DELIVERED'",
        buyerId,
        productId
    );

    return result[0]["count"].as<long long>() > 0;
}
