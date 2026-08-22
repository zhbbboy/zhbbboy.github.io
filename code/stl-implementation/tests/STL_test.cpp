#include "stl_implementation/bbnS.h"

#include <cstring>

int main()
{
    bbnS::string value("hello");
    value.push_back(' ');
    value += "world";
    if (std::strcmp(value.c_str(), "hello world") != 0)
    {
        return 1;
    }

    value.insert(5, ',');
    if (std::strcmp(value.c_str(), "hello, world") != 0)
    {
        return 2;
    }

    value.erase(5, 1);
    if (value.find('w') != 6 || value.rfind("lo") != 3)
    {
        return 3;
    }

    bbnS::string copy(value);
    copy.resize(5);
    if (std::strcmp(copy.c_str(), "hello") != 0 || copy.size() != 5)
    {
        return 4;
    }

    return copy < value ? 0 : 5;
}
