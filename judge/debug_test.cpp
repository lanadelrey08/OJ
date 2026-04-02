#include "include/sandbox.h"
#include <iostream>

int main() {
    std::cout << "Testing sandbox..." << std::endl;
    
    SandboxConfig config;
    config.time_limit_ms = 1000;
    config.memory_limit_mb = 128;
    config.output_limit_mb = 10;
    config.enable_network = false;
    config.enable_file_system = false;
    
    Sandbox sandbox(config);
    
    // 测试不同的命令
    std::string commands[] = {
        "echo hello",
        "cmd /c echo hello",
        "dir"
    };
    
    for (const auto& cmd : commands) {
        std::cout << "\nTesting command: " << cmd << std::endl;
        SandboxResult result = sandbox.execute(cmd, "");
        std::cout << "Command output: " << result.output << std::endl;
        std::cout << "Command error: " << result.error << std::endl;
        std::cout << "Exit code: " << result.exit_code << std::endl;
        std::cout << "Runtime: " << result.runtime_ms << "ms" << std::endl;
    }
    
    return 0;
}