#pragma once

#include <string>

struct Review
{
    long long id;
    long long productId;
    long long userId;
    int rating;
    std::string comment;
};