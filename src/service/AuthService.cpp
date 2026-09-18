#include "AuthService.h"

#include "../repository/UserRepository.h"
#include "../util/PasswordUtil.h"

bool AuthService::registerUser(
    const std::string& name,
    const std::string& email,
    const std::string& password,
    const std::string& role)
{
    if (name.empty() ||
        email.empty() ||
        password.empty())
    {
        return false;
    }

    if (role != "BUYER" && role != "SELLER")
    {
        return false;
    }

    UserRepository repository;

    auto existingUser =
        repository.findPasswordHashByEmail(email);

    if (existingUser.has_value())
    {
        return false;
    }

    const std::string passwordHash =
        PasswordUtil::hashPassword(password);

    return repository.createUser(
        name,
        email,
        passwordHash,
        role
    );
}

bool AuthService::loginUser(
    const std::string& email,
    const std::string& password)
{
    if (email.empty() || password.empty())
    {
        return false;
    }

    UserRepository repository;

    auto passwordHash =
        repository.findPasswordHashByEmail(email);

    if (!passwordHash.has_value())
    {
        return false;
    }

    return PasswordUtil::verifyPassword(
        password,
        passwordHash.value()
    );
}