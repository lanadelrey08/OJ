#include "submission_handler.h"
#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/log.h"

#include <cstdlib>
#include <iostream>

namespace {
void shutdownAtExit() {
    oj::shutdownInfrastructure();
}
}  // namespace

int main() {
    try {
        oj::AppConfig cfg = oj::loadConfigFromEnv();
        oj::initInfrastructure(cfg);
        std::atexit(shutdownAtExit);

        SubmissionHandler handler;
        std::cout << "Judge server starting on port " << cfg.judge_port << "..." << std::endl;
        OJ_LOG_INFO("judge_engine process started on port " + std::to_string(cfg.judge_port));
        handler.startServer(static_cast<uint16_t>(cfg.judge_port));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
