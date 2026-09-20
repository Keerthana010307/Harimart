#pragma once

#include "../model/Review.h"
#include <vector>
#include <string>

class ReviewService
{
public:
    bool addReview(
        long long productId,
        long long userId,
        int rating,
        const std::string& comment
    );

    std::vector<Review> getProductReviews(
        long long productId
    );
};