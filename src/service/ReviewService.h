#pragma once

#include "../model/Review.h"
#include <vector>
#include <string>

struct ReviewResult
{
    bool success;
    std::string errorMessage;
};

class ReviewService
{
public:
    ReviewResult addReview(
        long long productId,
        long long userId,
        int rating,
        const std::string& comment
    );

    std::vector<Review> getProductReviews(
        long long productId
    );
};