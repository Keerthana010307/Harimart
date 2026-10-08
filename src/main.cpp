#include <drogon/drogon.h>
#include <iostream>
#include <exception>

int main()
{
    try
    {
        std::cout << "STEP 1: Program started" << std::endl;

        drogon::app().loadConfigFile("config.json");

        // Enable CORS for frontend
        drogon::app().registerPreSendingAdvice(
            [](const drogon::HttpRequestPtr& req,
               const drogon::HttpResponsePtr& resp)
            {
                resp->addHeader(
                    "Access-Control-Allow-Origin",
                    "http://127.0.0.1:5500"
                );

                resp->addHeader(
                    "Access-Control-Allow-Credentials",
                    "true"
                );

                resp->addHeader(
                    "Access-Control-Allow-Headers",
                    "Content-Type"
                );

                resp->addHeader(
                    "Access-Control-Allow-Methods",
                    "GET, POST, PUT, DELETE, OPTIONS"
                );
            });

        // Handle CORS preflight requests
        drogon::app().registerSyncAdvice(
            [](const drogon::HttpRequestPtr& req)
            -> drogon::HttpResponsePtr
            {
                if (req->method() == drogon::Options)
{
    auto response =
        drogon::HttpResponse::newHttpResponse();

    response->setStatusCode(drogon::k200OK);

    response->addHeader(
        "Access-Control-Allow-Origin",
        "http://127.0.0.1:5500"
    );

    response->addHeader(
        "Access-Control-Allow-Credentials",
        "true"
    );

    response->addHeader(
        "Access-Control-Allow-Headers",
        "Content-Type"
    );

    response->addHeader(
        "Access-Control-Allow-Methods",
        "GET, POST, PUT, DELETE, OPTIONS"
    );

    return response;
}

                return nullptr;
            });

        std::cout << "STEP 2: Config loaded" << std::endl;

        drogon::app().run();

        std::cout << "STEP 3: Server stopped normally" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "UNKNOWN ERROR" << std::endl;
        return 1;
    }

    return 0;
}