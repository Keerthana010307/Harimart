#include "CartService.h"
#include "../repository/CartRepository.h"

bool CartService::addToCart(
    long long userId,
    long long productId,
    int quantity)
{
    if (userId <= 0 || productId <= 0 || quantity <= 0)
    {
        return false;
    }

    CartRepository repository;

    return repository.addOrUpdateItem(
        userId,
        productId,
        quantity
    );
}

bool CartService::updateCart(
    long long userId,
    long long productId,
    int quantity)
{
    if (userId <= 0 || productId <= 0 || quantity <= 0)
    {
        return false;
    }

    CartRepository repository;

    return repository.updateQuantity(
        userId,
        productId,
        quantity
    );
}

bool CartService::removeFromCart(
    long long userId,
    long long productId)
{
    if (userId <= 0 || productId <= 0)
    {
        return false;
    }

    CartRepository repository;

    return repository.removeItem(
        userId,
        productId
    );
}

std::vector<CartItem> CartService::getCart(
    long long userId)
{
    if (userId <= 0)
    {
        return {};
    }

    CartRepository repository;

    return repository.getCartItems(userId);
}