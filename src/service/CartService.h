#pragma once

#include "../model/CartItem.h"
#include <vector>

class CartService
{
public:
    bool addToCart(
        long long userId,
        long long productId,
        int quantity
    );

    bool updateCart(
        long long userId,
        long long productId,
        int quantity
    );

    bool removeFromCart(
        long long userId,
        long long productId
    );

    std::vector<CartItem> getCart(
        long long userId
    );
};