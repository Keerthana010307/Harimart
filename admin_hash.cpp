#include "src/util/PasswordUtil.h"
#include <iostream>

int main()
{
    std::cout << PasswordUtil::hashPassword("hari@2303") << std::endl;
    return 0;
}
