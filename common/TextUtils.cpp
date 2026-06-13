#include "TextUtils.h"
#include <boost/locale.hpp>
#include <cctype>

namespace {

    boost::locale::generator gen;
    std::locale loc = gen("en_US.UTF-8");
}

namespace TextUtils {

std::string toLower(const std::string& s) {

    return boost::locale::to_lower(s, loc);
}

std::map<std::string, int> wordFrequencies(const std::string& text) {
    std::map<std::string, int> freq;
    std::string word;


    auto flush = [&]() {
        if (word.size() >= 3 && word.size() <= 32) {
            freq[toLower(word)]++;
        }
        word.clear();
    };

    for (unsigned char ch : text) {
    
        if (std::isalnum(ch)) {
            word += static_cast<char>(ch);
        } else {
            flush();
        }
    }
    flush();

    return freq;
}

}
