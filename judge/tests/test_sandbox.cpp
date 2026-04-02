#include "sandbox.h"
#include <iostream>

int main() {
    std::cout << "Testing sandbox..." << std::endl;
    
    // 测试正常执行
    SandboxConfig config;
    config.time_limit_ms = 1000;
    config.memory_limit_mb = 128;
    config.output_limit_mb = 10;
    config.enable_network = false;
    config.enable_file_system = false;
    
    Sandbox sandbox(config);
    
    // 测试执行简单命令
    SandboxResult result = sandbox.execute("echo hello", "");
    std::cout << "Command output: " << result.output << std::endl;
    std::cout << "Command error output: " << result.error << std::endl;
    std::cout << "Exit code: " << result.exit_code << std::endl;
    std::cout << "Runtime: " << result.runtime_ms << "ms" << std::endl;
    
    return 0;
}