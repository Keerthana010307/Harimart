#include "ReviewRepository.h"
#include "Database.h"

bool ReviewRepository::createReview(
    long long productId,
    long long userId,
    int rating,
    const std::string& comment)
{
    auto dbClient = Database::getClient();

    try
    {
        dbClient->execSqlSync(
            "INSERT INTO reviews "
            "(product_id, user_id, rating, comment) "
            "VALUES ($1, $2, $3, $4)",
            productId,
            userId,
            rating,
            comment
        );

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::vector<Review> ReviewRepository::getReviewsByProduct(
    long long productId)
{
    auto dbClient = Database::getClient();

    std::vector<Review> reviews;

    auto result = dbClient->execSqlSync(
        "SELECT id, product_id, user_id, rating, "
        "COALESCE(comment, '') AS comment "
        "FROM reviews "
        "WHERE product_id = $1 "
        "ORDER BY id DESC",
        productId
    );

    for (const auto& row : result)
    {
        Review review;

        review.id = row["id"].as<long long>();
        review.productId = row["product_id"].as<long long>();
        review.userId = row["user_id"].as<long long>();
        review.rating = row["rating"].as<int>();
        review.comment = row["comment"].as<std::string>();

        reviews.push_back(review);
    }

    return reviews;
}

bool ReviewRepository::hasUserReviewedProduct(
    long long productId,
    long long userId)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT COUNT(*) AS count "
        "FROM reviews "
        "WHERE product_id = $1 AND user_id = $2",
        productId,
        userId
    );

    return result[0]["count"].as<long long>() > 0;
}