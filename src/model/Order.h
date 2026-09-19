#pragma once

#include <string>

struct Order
{
    long long id;
    long long buyerId;
    std::string status;
    long long totalAmountCents;
};