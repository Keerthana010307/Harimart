#include "UserRepository.h"

#include "Database.h"

bool UserRepository::createUser(
    const std::string& name,
    const std::string& email,
    const std::string& passwordHash,
    const std::string& role)
{
    auto dbClient = Database::getClient();

    dbClient->execSqlSync(
        "INSERT INTO users (name, email, password_hash, role) "
        "VALUES ($1, $2, $3, $4)",
        name,
        email,
        passwordHash,
        role
    );

    return true;
}

std::optional<std::string> UserRepository::findPasswordHashByEmail(
    const std::string& email)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT password_hash FROM users WHERE email = $1",
        email
    );

    if (result.empty())
    {
        return std::nullopt;
    }

    return result[0]["password_hash"].as<std::string>();
}
std::optional<long long> UserRepository::findUserIdByEmail(
    const std::string& email)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT id FROM users WHERE email = $1",
        email
    );

    if (result.empty())
    {
        return std::nullopt;
    }

    return result[0]["id"].as<long long>();
}

std::optional<std::string> UserRepository::findRoleByEmail(
    const std::string& email)
{
    auto dbClient = Database::getClient();

    auto result = dbClient->execSqlSync(
        "SELECT role FROM users WHERE email = $1",
        email
    );

    if (result.empty())
    {
        return std::nullopt;
    }

    return result[0]["role"].as<std::string>();
}