#include "ProductService.h"
#include "../repository/ProductRepository.h"

bool ProductService::createProduct(
    long long sellerId,
    const std::string& name,
    const std::string& description,
    long long priceCents,
    int stock)
{
    if (sellerId <= 0 ||
        name.empty() ||
        priceCents < 0 ||
        stock < 0)
    {
        return false;
    }

    ProductRepository repository;

    return repository.createProduct(
        sellerId,
        name,
        description,
        priceCents,
        stock
    );
}

std::vector<Product> ProductService::getAllProducts()
{
    ProductRepository repository;

    return repository.getAllProducts();
}

std::optional<Product> ProductService::getProductById(
    long long id)
{
    if (id <= 0)
    {
        return std::nullopt;
    }

    ProductRepository repository;

    return repository.getProductById(id);
}

bool ProductService::updateProduct(
    long long id,
    long long sellerId,
    const std::string& name,
    const std::string& description,
    long long priceCents,
    int stock)
{
    if (id <= 0 ||
        sellerId <= 0 ||
        name.empty() ||
        priceCents < 0 ||
        stock < 0)
    {
        return false;
    }

    ProductRepository repository;

    return repository.updateProduct(
        id,
        sellerId,
        name,
        description,
        priceCents,
        stock
    );
}

bool ProductService::deleteProduct(
    long long id,
    long long sellerId)
{
    if (id <= 0 || sellerId <= 0)
    {
        return false;
    }

    ProductRepository repository;

    return repository.deleteProduct(
        id,
        sellerId
    );
}

bool ProductService::adminDeleteProduct(
    long long id)
{
    if (id <= 0)
    {
        return false;
    }

    ProductRepository repository;

    return repository.adminDeleteProduct(id);
}