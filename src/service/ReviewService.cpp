#include "ReviewService.h"
#include "../repository/ReviewRepository.h"
#include "../repository/OrderRepository.h"

ReviewResult ReviewService::addReview(
    long long productId,
    long long userId,
    int rating,
    const std::string& comment)
{
    if (rating < 1 || rating > 5)
    {
        return {false, "Rating must be between 1 and 5"};
    }

    OrderRepository orderRepository;

    if (!orderRepository.hasCompletedPurchase(userId, productId))
    {
        return {false, "You must purchase and receive this product before reviewing"};
    }

    ReviewRepository repository;

    if (repository.hasUserReviewedProduct(productId, userId))
    {
        return {false, "You have already reviewed this product"};
    }

    bool created = repository.createReview(
        productId, userId, rating, comment
    );

    if (!created)
    {
        return {false, "Failed to save review. Please try again."};
    }

    return {true, ""};
}

std::vector<Review> ReviewService::getProductReviews(
    long long productId)
{
    ReviewRepository repository;
    return repository.getReviewsByProduct(productId);
}