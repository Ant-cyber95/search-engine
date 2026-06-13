#pragma once
#include <pqxx/pqxx>
#include <string>
#include <map>
#include <mutex>
#include <vector>
#include <utility>

class Database {
public:

    explicit Database(const std::string& connStr);


    void createTables();


    void saveDocument(const std::string& path,
                      const std::map<std::string, int>& words);


    std::vector<std::pair<std::string,int>>
        search(const std::vector<std::string>& words, int limit = 10);

private:
    pqxx::connection conn_;
    std::mutex mtx_;
};
