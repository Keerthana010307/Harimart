#include "ReviewService.h"
#include "../repository/ReviewRepository.h"

bool ReviewService::addReview(
    long long productId,
    long long userId,
    int rating,
    const std::string& comment)
{
    if (rating < 1 || rating > 5)
    {
        return false;
    }

    ReviewRepository repository;

    if (repository.hasUserReviewedProduct(productId, userId))
    {
        return false;
    }

    return repository.createReview(
        productId,
        userId,
        rating,
        comment
    );
}

std::vector<Review> ReviewService::getProductReviews(
    long long productId)
{
    ReviewRepository repository;

    return repository.getReviewsByProduct(productId);
}