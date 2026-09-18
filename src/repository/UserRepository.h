#pragma once

#include <string>
#include <optional>

class UserRepository
{
public:
    bool createUser(
        const std::string& name,
        const std::string& email,
        const std::string& passwordHash,
        const std::string& role
    );

    std::optional<std::string> findPasswordHashByEmail(
        const std::string& email
    );

    std::optional<std::string> findRoleByEmail(
        const std::string& email
    );
};