#include "OrderService.h"
#include "../repository/OrderRepository.h"
#include "../repository/CartRepository.h"

long long OrderService::createOrder(long long buyerId)
{
    if (buyerId <= 0)
    {
        return 0;
    }

    CartRepository cartRepository;
    auto cartItems = cartRepository.getCartItems(buyerId);

    if (cartItems.empty())
    {
        return 0;
    }

    long long totalAmountCents = 0;

    for (const auto& item : cartItems)
    {
        totalAmountCents +=
            item.priceCents * item.quantity;
    }

    OrderRepository orderRepository;

    long long orderId =
        orderRepository.createOrder(
            buyerId,
            totalAmountCents
        );

    for (const auto& item : cartItems)
    {
        orderRepository.addOrderItem(
            orderId,
            item.productId,
            item.quantity,
            item.priceCents
        );
    }
    
    



    return orderId;
}

std::vector<Order> OrderService::getOrdersByBuyer(
    long long buyerId)
{
    if (buyerId <= 0)
    {
        return {};
    }

    OrderRepository repository;

    return repository.getOrdersByBuyer(buyerId);
}
bool OrderService::confirmOrder(long long orderId)
{
    if (orderId <= 0)
    {
        return false;
    }

    OrderRepository repository;

    return repository.updateOrderStatus(
        orderId,
        "CONFIRMED"
    );
}
bool OrderService::isOrderOwnedByBuyer(
    long long orderId,
    long long buyerId)
{
    OrderRepository repository;

    return repository.isOrderOwnedByBuyer(
        orderId,
        buyerId
    );
}
std::vector<Order> OrderService::getOrdersBySeller(
    long long sellerId)
{
    if (sellerId <= 0)
    {
        return {};
    }

    OrderRepository repository;

    return repository.getOrdersBySeller(sellerId);
}
bool OrderService::updateOrderStatus(
    long long orderId,
    const std::string& status)
{
    if (orderId <= 0 || status.empty())
    {
        return false;
    }

    if (status != "CONFIRMED" &&
        status != "SHIPPED" &&
        status != "DELIVERED" &&
        status != "CANCELLED")
    {
        return false;
    }

    OrderRepository repository;

    return repository.updateOrderStatus(
        orderId,
        status
    );
}