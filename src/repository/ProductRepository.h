#pragma once

#include "../model/Product.h"

#include <optional>
#include <string>
#include <vector>

class ProductRepository
{
public:
    bool createProduct(
        long long sellerId,
        const std::string& name,
        const std::string& description,
        long long priceCents,
        int stock
    );

    std::vector<Product> getAllProducts();

    std::optional<Product> getProductById(long long id);

    bool updateProduct(
        long long  id,
        long long sellerId,
        const std::string& name,
        const std::string& description,
        long long priceCents,
        int stock
    );

    bool deleteProduct(
        long long  id,
        long long sellerId
    );
};