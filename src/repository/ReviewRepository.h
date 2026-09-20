#pragma once

#include "../model/Review.h"
#include <vector>
#include <optional>

class ReviewRepository
{
public:
    bool createReview(
        long long productId,
        long long userId,
        int rating,
        const std::string& comment
    );

    std::vector<Review> getReviewsByProduct(
        long long productId
    );

    bool hasUserReviewedProduct(
        long long productId,
        long long userId
    );
};