#include <drogon/drogon.h>
#include <iostream>
#include <exception>

int main()
{
    try
    {
        std::cout << "STEP 1: Program started" << std::endl;

        drogon::app().loadConfigFile("config.json");

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