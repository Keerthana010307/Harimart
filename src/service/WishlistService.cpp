#include "WishlistService.h"
#include "../repository/WishlistRepository.h"

bool WishlistService::addToWishlist(long long userId, long long productId)
{
    if (userId <= 0 || productId <= 0) return false;
    WishlistRepository repository;
    return repository.addItem(userId, productId);
}

bool WishlistService::removeFromWishlist(long long userId, long long productId)
{
    if (userId <= 0 || productId <= 0) return false;
    WishlistRepository repository;
    return repository.removeItem(userId, productId);
}

std::vector<WishlistItem> WishlistService::getWishlist(long long userId)
{
    if (userId <= 0) return {};
    WishlistRepository repository;
    return repository.getItems(userId);
}
