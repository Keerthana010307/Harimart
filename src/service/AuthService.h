#pragma once

#include <string>

class AuthService
{
public:
    bool registerUser(
        const std::string& name,
        const std::string& email,
        const std::string& password,
        const std::string& role
    );

    bool loginUser(
        const std::string& email,
        const std::string& password
    );
};