#include <vector>
#include <string>

std::vector<std::string> split_string(const std::string& s, const std::string& delims)
{
    std::vector<std::string> tokens;

    std::string::size_type lastPos = s.find_first_not_of(delims, 0);
    std::string::size_type pos     = s.find_first_of(delims, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        tokens.push_back(s.substr(lastPos, pos - lastPos));
        lastPos = s.find_first_not_of(delims, pos);
        pos = s.find_first_of(delims, lastPos);
    }

    return tokens;
}