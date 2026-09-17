#pragma once

#include <drogon/drogon.h>

class Database
{
public:
    static drogon::orm::DbClientPtr getClient();
};