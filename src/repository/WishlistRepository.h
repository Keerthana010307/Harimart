#pragma once

#include "../model/WishlistItem.h"
#include <vector>

class WishlistRepository
{
public:
    bool addItem(long long userId, long long productId);
    bool removeItem(long long userId, long long productId);
    std::vector<WishlistItem> getItems(long long userId);
    bool hasItem(long long userId, long long productId);
};
