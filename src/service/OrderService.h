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
    bool confirmOrder(long long orderId);
};