#include <vector>
#include <string>
#include <filesystem>

#include "util.hpp"

namespace fs = std::filesystem;

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

    // I hate carriage return
    for (int i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].ends_with('\r'))
        {
            tokens[i].pop_back();
        }
    }
    return tokens;
}

long long get_modified_time_u64(fs::path path)
{
    std::filesystem::file_time_type lwt = fs::last_write_time(path);
    #ifdef __APPLE__
        auto sys_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lwt);
        auto tse = sys_time.time_since_epoch();
    #else
        auto sys_time = std::chrono::clock_cast<std::chrono::system_clock>(lwt);
        std::filesystem::_File_time_clock::duration tse = sys_time.time_since_epoch();
    #endif
    std::chrono::seconds dc = std::chrono::duration_cast<std::chrono::seconds>(tse);
    return dc.count(); 
}

RECOMP_DLL_FUNC(get_current_time)
{
    RECOMP_RETURN(u32, (u32)(time(nullptr) & 0xFFFFFF));
}