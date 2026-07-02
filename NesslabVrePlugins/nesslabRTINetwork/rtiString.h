#ifndef stringUtil_H_
#define stringUtil_H_

#include <string>
#include <sstream>

// Convert narrow C string to wide string
inline std::wstring DtToWString(const char* in_val)
{
    std::wstring temp;
    while (*in_val != '\0')
        temp += *in_val++;
    return temp;
}

// Convert narrow string to wide string
inline std::string DtToString(const std::wstring& in_val)
{
    std::string temp;
    std::wstring::const_iterator b = in_val.begin();
    const std::wstring::const_iterator e = in_val.end();
    while (b != e)
    {
        temp += static_cast<char>(*b);
        ++b;
    }
    return temp;
}

#endif

