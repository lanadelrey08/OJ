#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/json_error.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

constexpr const char* kEnvMysqlHost = "OJ_MYSQL_HOST";
constexpr const char* kEnvMysqlPort = "OJ_MYSQL_PORT";
constexpr const char* kEnvMysqlUser = "OJ_MYSQL_USER";
constexpr const char* kEnvRedisDb = "OJ_REDIS_DB";
constexpr const char* kEnvLogLevel = "OJ_LOG_LEVEL";
constexpr const char* kBootstrapStartLog = "OJ public infrastructure starting";

struct ScopedEnvVar {
    std::string key;
    std::string original_value;
    bool had_original{false};

    ScopedEnvVar(std::string env_key, std::string value) : key(std::move(env_key)) {
        if (const char* existing = std::getenv(key.c_str())) {
            had_original = true;
            original_value = existing;
        }
#if defined(_WIN32)
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 1);
#endif
    }

    ~ScopedEnvVar() {
#if defined(_WIN32)
        _putenv_s(key.c_str(), had_original ? original_value.c_str() : "");
#else
        if (had_original) {
            setenv(key.c_str(), original_value.c_str(), 1);
        } else {
            unsetenv(key.c_str());
        }
#endif
    }
};

bool expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << std::endl;
        return false;
    }
    std::cout << "[PASS] " << message << std::endl;
    return true;
}

std::filesystem::path makeTempLogPath(const std::string& tag) {
    const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("oj_common_" + tag + "_" + std::to_string(now) + ".log");
}

std::string readWholeFile(const std::filesystem::path& p) {
    std::ifstream in(p);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

bool testLoadConfigFromEnv() {
    const std::string expected_mysql_host = "192.168.0.10";
    const std::string expected_mysql_port = "3307";
    const std::string expected_mysql_user = "judge_user";
    const std::string expected_redis_db = "5";
    const std::string expected_log_level = "debug";

    ScopedEnvVar mysql_host(kEnvMysqlHost, expected_mysql_host);
    ScopedEnvVar mysql_port(kEnvMysqlPort, expected_mysql_port);
    ScopedEnvVar mysql_user(kEnvMysqlUser, expected_mysql_user);
    ScopedEnvVar redis_db(kEnvRedisDb, expected_redis_db);
    ScopedEnvVar log_level(kEnvLogLevel, expected_log_level);

    const oj::AppConfig cfg = oj::loadConfigFromEnv();
    return expect(cfg.mysql_host == expected_mysql_host, "config reads mysql host from env") &&
           expect(cfg.mysql_port == static_cast<unsigned>(std::stoul(expected_mysql_port)),
                  "config reads mysql port from env") &&
           expect(cfg.mysql_user == expected_mysql_user, "config reads mysql user from env") &&
           expect(cfg.redis_db == std::stoi(expected_redis_db), "config reads redis db from env") &&
           expect(cfg.log_level == expected_log_level, "config reads log level from env");
}

bool testMakeErrorJson() {
    const std::string input_code = "bad\ncode";
    const std::string input_message = "line1\n\"quoted\"\\tail\t";
    const std::string expected =
        "{\"error_code\":\"bad\\ncode\",\"message\":\"line1\\n\\\"quoted\\\"\\\\tail\\t\"}";
    const std::string json = oj::makeErrorJson(input_code, input_message);
    return expect(json == expected, "error json escapes control characters correctly");
}

bool testLoggerWritesFile() {
    const auto log_path = makeTempLogPath("logger");
    std::filesystem::remove(log_path);

    oj::Logger::instance().init(oj::LogLevel::Info, log_path.string());
    oj::Logger::instance().info("common smoke logger test");
    oj::Logger::instance().init(oj::LogLevel::Info);

    const std::string content = readWholeFile(log_path);
    const bool ok = expect(std::filesystem::exists(log_path), "logger creates output file") &&
                    expect(content.find("common smoke logger test") != std::string::npos,
                           "logger writes message to file");
    std::filesystem::remove(log_path);
    return ok;
}

bool testBootstrapWithoutOptionalBackends() {
    const auto log_path = makeTempLogPath("bootstrap");
    std::filesystem::remove(log_path);

    oj::AppConfig cfg;
    cfg.log_level = "debug";
    cfg.log_file = log_path.string();
    oj::initInfrastructure(cfg);

    const bool mysql_disabled = expect(!oj::MysqlConnectionPool::instance().available(),
                                       "mysql pool stays disabled when mysql support is off");
    const bool redis_disabled = expect(!oj::RedisCache::instance().connected(),
                                       "redis cache stays disabled when redis support is off");

    oj::shutdownInfrastructure();
    oj::Logger::instance().init(oj::LogLevel::Info);

    const std::string content = readWholeFile(log_path);
    const bool log_written =
        expect(content.find(kBootstrapStartLog) != std::string::npos, "bootstrap writes startup log");
    std::filesystem::remove(log_path);
    return mysql_disabled && redis_disabled && log_written;
}

}  // namespace

int main() {
    bool ok = true;
    ok = testLoadConfigFromEnv() && ok;
    ok = testMakeErrorJson() && ok;
    ok = testLoggerWritesFile() && ok;
    ok = testBootstrapWithoutOptionalBackends() && ok;

    if (!ok) {
        std::cerr << "Common smoke tests failed." << std::endl;
        return 1;
    }
    std::cout << "Common smoke tests passed." << std::endl;
    return 0;
}
