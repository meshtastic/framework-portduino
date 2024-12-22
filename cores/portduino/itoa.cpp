#include <itoa.h>
#include <string>
#include <stdexcept>
#include <stdio.h>


std::string radixToFmtString(int const radix)
{
    if (radix == 8)  return std::string("%o");
    else if (radix == 10) return std::string("%d");
    else if (radix == 16) return std::string("%X");
    else throw std::runtime_error("Invalid radix.");
}

char * itoa(int value, char * str, int radix)
{
    sprintf(str, radixToFmtString(radix).c_str(), value);
    return str;
}

char * ltoa(long value, char * str, int radix)
{
    sprintf(str, radixToFmtString(radix).c_str(), value);
    return str;
}

char * utoa(unsigned value, char *str, int radix)
{
    sprintf(str, radixToFmtString(radix).c_str(), value);
    return str;
}

char * ultoa(unsigned long value, char * str, int radix)
{
    sprintf(str, radixToFmtString(radix).c_str(), value);
    return str;
}
