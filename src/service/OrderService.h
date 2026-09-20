#pragma once

#include "../model/Order.h"
#include <vector>

class OrderService
{
public:
    long long createOrder(long long buyerId);

    std::vector<Order> getOrdersByBuyer(
        long long buyerId
    );

    std::vector<Order> getOrdersBySeller(
        long long sellerId
    );

    bool confirmOrder(long long orderId);
    bool isOrderOwnedByBuyer(
    long long orderId,
    long long buyerId
);
    bool updateOrderStatus(
    long long orderId,
    const std::string& status
);
};