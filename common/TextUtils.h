#pragma once
#include <string>
#include <map>

namespace TextUtils {

    std::string toLower(const std::string& s);

    std::map<std::string, int> wordFrequencies(const std::string& text);
}
