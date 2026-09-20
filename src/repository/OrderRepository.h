#pragma once

#include "../model/Order.h"
#include "../model/OrderItem.h"
#include <vector>
#include <string>

class OrderRepository
{
public:
    long long createOrder(
        long long buyerId,
        long long totalAmountCents
    );

    bool addOrderItem(
        long long orderId,
        long long productId,
        int quantity,
        long long unitPriceCents
    );

    std::vector<Order> getOrdersByBuyer(
        long long buyerId
    );

    std::vector<Order> getOrdersBySeller(
        long long sellerId
    );

    bool updateOrderStatus(
        long long orderId,
        const std::string& status
    );

    bool confirmOrder(long long orderId);
    
    bool isOrderOwnedByBuyer(
    long long orderId,
    long long buyerId
);
        

    bool hasCompletedPurchase(
        long long buyerId,
        long long productId
    );
};
