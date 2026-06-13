#include "../common/Config.h"
#include "../common/Database.h"
#include "../common/TextUtils.h"

#include <iostream>
#include <string>
#include <vector>


static std::vector<std::string> parseQuery(const std::string& line) {
    auto freq = TextUtils::wordFrequencies(line);
    std::vector<std::string> words;
    for (const auto& [w, _] : freq) {
        words.push_back(w);
    }
    if (words.size() > 4) {
        words.resize(4);
    }
    return words;
}

int main() {
    try {

        Config cfg = Config::load("config.ini");
        Database db(cfg.connectionString());


        std::cout << "Search query: ";
        std::string line;
        std::getline(std::cin, line);


        std::vector<std::string> words = parseQuery(line);
        if (words.empty()) {
            std::cout << "Empty query. Nothing to search.\n";
            return 0;
        }


        auto results = db.search(words, 10);


        if (results.empty()) {
            std::cout << "No results found.\n";
            return 0;
        }

        std::cout << "\nResults (rank\tpath):\n";
        for (const auto& [path, rank] : results) {
            std::cout << rank << "\t" << path << "\n";
        }

    } catch (const std::exception& e) {

        std::cerr << "Internal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
