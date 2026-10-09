#include <drogon/drogon.h>
#include <iostream>
#include <exception>

int main()
{
    try
    {
        std::cout << "HariMart: Starting server..." << std::endl;

        drogon::app().loadConfigFile("config.json");

        // CORS middleware - supports both same-origin and cross-origin
        drogon::app().registerPreSendingAdvice(
            [](const drogon::HttpRequestPtr& req,
               const drogon::HttpResponsePtr& resp)
            {
                auto origin = req->getHeader("Origin");

                // Allow same-origin (Drogon serves frontend) and dev origins
                if (origin.empty() ||
                    origin == "http://127.0.0.1:8080" ||
                    origin == "http://localhost:8080" ||
                    origin == "http://127.0.0.1:5500" ||
                    origin == "http://localhost:5500")
                {
                    if (!origin.empty())
                    {
                        resp->addHeader(
                            "Access-Control-Allow-Origin",
                            origin
                        );
                    }
                }

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

                    auto origin = req->getHeader("Origin");

                    if (!origin.empty())
                    {
                        response->addHeader(
                            "Access-Control-Allow-Origin",
                            origin
                        );
                    }

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

        std::cout << "HariMart: Config loaded, starting on port 8080" << std::endl;

        drogon::app().run();

        std::cout << "HariMart: Server stopped" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "HariMart ERROR: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "HariMart: Unknown error" << std::endl;
        return 1;
    }

    return 0;
}