#include "discussion_handler.h"
#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/log.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void shutdownAtExit() {
    oj::shutdownInfrastructure();
}

uint16_t loadPortFromEnv() {
#if defined(_WIN32)
    char* value = nullptr;
    size_t len = 0;
    if (_dupenv_s(&value, &len, "OJ_DISCUSSION_PORT") == 0 && value) {
        try {
            const int parsed = std::stoi(value);
            std::free(value);
            if (parsed > 0 && parsed <= 65535) {
                return static_cast<uint16_t>(parsed);
            }
        } catch (...) {
            std::free(value);
            return 8090;
        }
    }
    return 8090;
#else
    const char* value = std::getenv("OJ_DISCUSSION_PORT");
    if (!value) {
        return 8090;
    }
    try {
        const int parsed = std::stoi(value);
        if (parsed > 0 && parsed <= 65535) {
            return static_cast<uint16_t>(parsed);
        }
    } catch (...) {
    }
    return 8090;
#endif
}
}  // namespace

int main() {
    try {
        oj::AppConfig cfg = oj::loadConfigFromEnv();
        oj::initInfrastructure(cfg);
        std::atexit(shutdownAtExit);

        DiscussionHandler handler;
        const uint16_t port = loadPortFromEnv();
        std::cout << "Discussion server starting on port " << port << "..." << std::endl;
        OJ_LOG_INFO("discussion_server process started");
        handler.startServer(port);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
