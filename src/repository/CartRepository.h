#pragma once

#include "../model/CartItem.h"
#include <vector>

class CartRepository
{
public:
    bool addOrUpdateItem(
        long long userId,
        long long productId,
        int quantity
    );

    bool updateQuantity(
        long long userId,
        long long productId,
        int quantity
    );

    bool removeItem(
        long long userId,
        long long productId
    );

    std::vector<CartItem> getCartItems(
        long long userId
    );
};
