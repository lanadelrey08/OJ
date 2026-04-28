#pragma once

#include <string>

namespace oj {

inline constexpr const char* kDefaultMysqlHost = "127.0.0.1";
inline constexpr unsigned kDefaultMysqlPort = 3306;
inline constexpr const char* kDefaultMysqlUser = "oj";
inline constexpr const char* kDefaultMysqlDatabase = "myOJ";
inline constexpr unsigned kDefaultMysqlPoolMin = 2;
inline constexpr unsigned kDefaultMysqlPoolMax = 16;

inline constexpr const char* kDefaultRedisHost = "127.0.0.1";
inline constexpr unsigned kDefaultRedisPort = 6379;
inline constexpr int kDefaultRedisDb = 0;

inline constexpr const char* kDefaultLogLevel = "info";

struct AppConfig {
    std::string mysql_host = kDefaultMysqlHost;
    unsigned mysql_port = kDefaultMysqlPort;
    std::string mysql_user = kDefaultMysqlUser;
    std::string mysql_password;
    std::string mysql_database = kDefaultMysqlDatabase;
    unsigned mysql_pool_min = kDefaultMysqlPoolMin;
    unsigned mysql_pool_max = kDefaultMysqlPoolMax;

    std::string redis_host = kDefaultRedisHost;
    unsigned redis_port = kDefaultRedisPort;
    std::string redis_password;
    int redis_db = kDefaultRedisDb;

    std::string log_file;
    std::string log_level = kDefaultLogLevel;
};

AppConfig loadConfigFromEnv();

}  // namespace oj
