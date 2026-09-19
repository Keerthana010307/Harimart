#include "AdminAuthUtil.h"
#include "../repository/UserRepository.h"

bool isAdmin(const drogon::HttpRequestPtr& req)
{
    if (!req->session()->find("user_email"))
    {
        return false;
    }

    auto email = req->session()->get<std::string>("user_email");

    UserRepository repository;
    auto role = repository.findRoleByEmail(email);

    return role.has_value() && role.value() == "ADMIN";
}
