#pragma once

#include <string>

struct Product
{
    int id;
    int sellerId;
    std::string name;
    std::string description;
    long long priceCents;
    int stock;
};