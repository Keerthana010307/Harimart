#include <drogon/drogon.h>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <string>

int main()
{
    try
    {
        std::cout << "HariMart: Starting server..." << std::endl;

        drogon::app().loadConfigFile("config.json");

        // Detect HTTPS (Render sets the RENDER env var)
        bool isHttps = (std::getenv("RENDER") != nullptr)
                     || (std::getenv("RENDER_EXTERNAL_URL") != nullptr);

        if (isHttps)
        {
            std::cout << "HariMart: HTTPS mode detected (Render)"
                      << std::endl;
        }

        // CORS + cookie security middleware
        drogon::app().registerPreSendingAdvice(
            [isHttps](const drogon::HttpRequestPtr& req,
               const drogon::HttpResponsePtr& resp)
            {
                auto origin = req->getHeader("Origin");

                if (!origin.empty())
                {
                    resp->addHeader(
                        "Access-Control-Allow-Origin",
                        origin
                    );
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

                // On HTTPS (Render), ensure session cookies have
                // the Secure flag so the browser accepts them.
                if (isHttps)
                {
                    auto setCookie = resp->getHeader("Set-Cookie");
                    if (!setCookie.empty()
                        && setCookie.find("Secure") == std::string::npos)
                    {
                        resp->addHeader(
                            "Set-Cookie",
                            setCookie + "; Secure"
                        );
                    }
                }
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

        std::cout << "HariMart: Config loaded, launching server..."
                  << std::endl;

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