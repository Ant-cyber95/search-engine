#include "Indexer.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

Indexer::Indexer(const IndexerConfig& cfg, Database& db)
    : cfg_(cfg), db_(db), pool_(cfg.threads) {}

static std::string toLowerExt(std::string s) {
    if (!s.empty() && s.front()=='.') s.erase(0,1);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void Indexer::run() {
    int queued = 0;
    for (auto& entry : fs::recursive_directory_iterator(
             cfg_.startDir, fs::directory_options::skip_permission_denied))
    {
        if (!entry.is_regular_file()) continue;
        std::string ext = toLowerExt(entry.path().extension().string());
        if (std::find(cfg_.extensions.begin(), cfg_.extensions.end(), ext)
            == cfg_.extensions.end()) continue;

        std::string p = entry.path().string();
        pool_.enqueue([this, p]{ processFile(p); });
        ++queued;
    }
    std::cout << "Поставлено в очередь: " << queued << " файлов\n";
    pool_.wait();
}

void Indexer::processFile(const std::string& path) {
    try {
        std::ifstream f(path, std::ios::binary);
        if (!f) return;
        std::ostringstream ss; ss << f.rdbuf();
        auto freq = processor_.process(ss.str());
        if (freq.empty()) return;
        db_.saveDocument(path, freq);
        std::cout << "[OK] " << path << " (" << freq.size() << " слов)\n";
    } catch (const std::exception& e) {
        std::cerr << "[ERR] " << path << ": " << e.what() << "\n";
    }
}