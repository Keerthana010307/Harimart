#include "Database.h"

drogon::orm::DbClientPtr Database::getClient()
{
    return drogon::app().getDbClient();
}