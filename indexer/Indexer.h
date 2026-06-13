#pragma once
#include <string>
#include <mutex>
#include "Config.h"
#include "Database.h"
#include "TextProcessor.h"
#include "ThreadPool.h"

class Indexer {
public:
    Indexer(const IndexerConfig& cfg, Database& db);
    void run();
private:
    void processFile(const std::string& path);

    const IndexerConfig& cfg_;
    Database& db_;
    TextProcessor processor_;
    ThreadPool pool_;
};