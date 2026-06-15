
#include "TextUtils.h"
#include <boost/locale.hpp>
#include <cctype>

namespace {


const std::locale& getLocale() {
    static const std::locale loc = [] {
        boost::locale::generator gen;
        return gen("en_US.UTF-8");
    }();
    return loc;
}

} 

namespace TextUtils {

std::string toLower(const std::string& s) {
    try {
        return boost::locale::to_lower(s, getLocale());
    } catch (const std::exception&) {

        std::string out = s;
        for (auto& c : out)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return out;
    }
}

std::map<std::string, int> wordFrequencies(const std::string& text) {
    std::map<std::string, int> freq;
    std::string word;


    auto isWordByte = [](unsigned char ch) -> bool {
        return std::isalnum(ch) || ch >= 0x80;
    };


    auto flush = [&]() {
        if (word.size() >= 3 && word.size() <= 64) {
            freq[toLower(word)]++;
        }
        word.clear();
    };

    for (unsigned char ch : text) {
        if (isWordByte(ch)) {
            word += static_cast<char>(ch);
        } else {
            flush();
        }
    }
    flush();

    return freq;
}

}
