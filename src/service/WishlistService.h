#pragma once

#include "../model/WishlistItem.h"
#include <vector>

class WishlistService
{
public:
    bool addToWishlist(long long userId, long long productId);
    bool removeFromWishlist(long long userId, long long productId);
    std::vector<WishlistItem> getWishlist(long long userId);
};
