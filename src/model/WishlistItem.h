#pragma once

#include <string>

struct WishlistItem
{
    long long id;
    long long userId;
    long long productId;
    std::string productName;
    long long priceCents;
    std::string imageUrl;
    std::string category;
};
