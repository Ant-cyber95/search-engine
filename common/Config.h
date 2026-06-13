#pragma once
#include <string>
#include <vector>


struct Config {
    
    std::string host;       
    int         port = 5432;
    std::string dbname;     
    std::string user;       
    std::string password;   

    
    std::string startDir;                
    std::vector<std::string> extensions; 
    int threads = 4;                     

    
    static Config load(const std::string& path);

    
    std::string connectionString() const;
};
