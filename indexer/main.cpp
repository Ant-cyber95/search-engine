#include "../common/Config.h"
#include "../common/Database.h"
#include "../common/TextUtils.h"
#include "ThreadPool.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <atomic>

namespace fs = std::filesystem;


static std::string readFile(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}


static bool extAllowed(const fs::path& p,
                       const std::vector<std::string>& exts) {
    if (!p.has_extension()) return false;

    std::string e = p.extension().string();
    if (!e.empty() && e[0] == '.') e.erase(0, 1);


    std::transform(e.begin(), e.end(), e.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    return std::find(exts.begin(), exts.end(), e) != exts.end();
}

int main() {
    try {
    
        Config cfg = Config::load("config.ini");
        std::cout << "Config loaded. Indexing dir: " << cfg.startDir << "\n";

    
        Database db(cfg.connectionString());
        db.createTables();
        std::cout << "Tables ready.\n";


        ThreadPool pool(cfg.threads);
        std::atomic<int> count{0};


        for (const auto& entry :
                 fs::recursive_directory_iterator(cfg.startDir)) {

            if (!entry.is_regular_file()) continue;
            if (!extAllowed(entry.path(), cfg.extensions)) continue;

            fs::path path = entry.path();



            pool.enqueue([&db, path, &count] {
                try {
                    std::string text = readFile(path);
                    auto freq = TextUtils::wordFrequencies(text);
                    db.saveDocument(path.string(), freq);
                    std::cout << "Indexed: " << path.string() << "\n";
                    ++count;
                } catch (const std::exception& e) {
                    std::cerr << "Error on " << path.string()
                              << ": " << e.what() << "\n";
                }
            });
        }



    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Done.\n";
    return 0;
}
