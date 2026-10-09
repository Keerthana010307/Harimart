#pragma once

#include "../model/Product.h"

#include <optional>
#include <string>
#include <vector>

class ProductService
{
public:
    bool createProduct(
        long long sellerId,
        const std::string& name,
        const std::string& description,
        long long priceCents,
        int stock,
        const std::string& category,
        const std::string& imageUrl
    );

    std::vector<Product> getAllProducts();

    std::optional<Product> getProductById(long long id);

    bool updateProduct(
        long long id,
        long long sellerId,
        const std::string& name,
        const std::string& description,
        long long priceCents,
        int stock,
        const std::string& category,
        const std::string& imageUrl
    );

    bool deleteProduct(
        long long id,
        long long sellerId
    );

    bool adminDeleteProduct(long long id);
};