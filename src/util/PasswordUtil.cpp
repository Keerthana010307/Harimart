#include "PasswordUtil.h"

#include <sodium.h>
#include <stdexcept>

std::string PasswordUtil::hashPassword(const std::string& password)
{
    if (sodium_init() < 0)
    {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    char hash[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(
            hash,
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        throw std::runtime_error("Password hashing failed");
    }

    return std::string(hash);
}

bool PasswordUtil::verifyPassword(
    const std::string& password,
    const std::string& hash)
{
    if (sodium_init() < 0)
    {
        return false;
    }

    return crypto_pwhash_str_verify(
               hash.c_str(),
               password.c_str(),
               password.size()) == 0;
}