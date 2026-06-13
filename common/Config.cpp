#include "Config.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <sstream>
#include <stdexcept>


static std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, sep)) {
        
        size_t a = item.find_first_not_of(" \t");
        size_t b = item.find_last_not_of(" \t");
        if (a != std::string::npos)
            out.push_back(item.substr(a, b - a + 1));
    }
    return out;
}

Config Config::load(const std::string& path) {
    boost::property_tree::ptree pt;
   
    boost::property_tree::ini_parser::read_ini(path, pt);

    Config c;
   
    c.host     = pt.get<std::string>("database.host");
    c.port     = pt.get<int>("database.port");
    c.dbname   = pt.get<std::string>("database.dbname");
    c.user     = pt.get<std::string>("database.user");
    c.password = pt.get<std::string>("database.password");

    c.startDir   = pt.get<std::string>("indexer.start_dir");
    c.extensions = split(pt.get<std::string>("indexer.extensions"), ',');
   
    c.threads    = pt.get<int>("indexer.threads", 4);

    if (c.extensions.empty())
        throw std::runtime_error("В config.ini не заданы расширения файлов");

    return c;
}

std::string Config::connectionString() const {
    std::ostringstream os;
    os << "host="     << host
       << " port="    << port
       << " dbname="  << dbname
       << " user="    << user
       << " password=" << password;
    return os.str();
}
