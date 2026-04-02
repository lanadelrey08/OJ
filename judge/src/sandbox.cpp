/**
 * @file sandbox.cpp
 * @brief 沙箱环境实现
 * @author OJ Team
 * @date 2024-01-01
 */
#include "sandbox.h"
#include <iostream>
#include <windows.h>
#include <process.h>
#include <string>
#include <sstream>
#include <vector>

/**
 * @brief 构造函数
 * @param config 沙箱配置
 */
Sandbox::Sandbox(const SandboxConfig& config) : config_(config) {
}

/**
 * @brief 设置资源限制
 * @return 是否设置成功
 */
bool Sandbox::setupResourceLimits() {
    // 在Windows中，资源限制通过Job对象实现
    return true;
}

/**
 * @brief 创建隔离环境
 * @return 是否创建成功
 */
bool Sandbox::createIsolatedEnvironment() {
    // 在Windows中，隔离环境可以通过AppContainer或其他沙箱技术实现
    return true;
}

/**
 * @brief 执行命令
 * @param command 命令字符串
 * @param input 输入数据
 * @return 执行结果
 */
SandboxResult Sandbox::execute(const std::string& command, const std::string& input) {
    SandboxResult result;
    result.exit_code = -1;
    result.runtime_ms = 0;
    result.memory_kb = 0;
    result.timeout = false;
    result.memory_exceeded = false;
    
    // 创建进程信息结构
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    // 创建管道用于捕获输出
    HANDLE hOutputRead, hOutputWrite;
    HANDLE hErrorRead, hErrorWrite;
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    if (!CreatePipe(&hOutputRead, &hOutputWrite, &saAttr, 0)) {
        result.error = "Failed to create output pipe";
        return result;
    }
    
    if (!CreatePipe(&hErrorRead, &hErrorWrite, &saAttr, 0)) {
        result.error = "Failed to create error pipe";
        CloseHandle(hOutputRead);
        CloseHandle(hOutputWrite);
        return result;
    }
    
    // 设置进程的标准输出和错误
    si.hStdOutput = hOutputWrite;
    si.hStdError = hErrorWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;
    
    // 处理命令行 - 在Windows中，需要使用cmd.exe执行内置命令
    std::string full_command = "cmd.exe /c " + command;
    
    // CreateProcessA需要可修改的字符串
    char* cmd_line = new char[full_command.size() + 1];
    strcpy(cmd_line, full_command.c_str());
    
    // 启动进程
    BOOL bSuccess = CreateProcessA(
        NULL,                  // 应用程序名称
        cmd_line,              // 命令行
        NULL,                  // 进程安全属性
        NULL,                  // 线程安全属性
        TRUE,                  // 继承句柄
        0,                     // 创建标志
        NULL,                  // 环境变量
        NULL,                  // 当前目录
        &si,                   // 启动信息
        &pi                    // 进程信息
    );
    
    if (!bSuccess) {
        DWORD error_code = GetLastError();
        char error_msg[256];
        FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            error_msg, sizeof(error_msg), NULL
        );
        result.error = "Failed to create process: " + std::string(error_msg);
        delete[] cmd_line;
        CloseHandle(hOutputRead);
        CloseHandle(hOutputWrite);
        CloseHandle(hErrorRead);
        CloseHandle(hErrorWrite);
        return result;
    }
    
    // 释放命令行内存
    delete[] cmd_line;
    
    // 关闭不需要的管道句柄
    CloseHandle(hOutputWrite);
    CloseHandle(hErrorWrite);
    
    // 等待进程完成或超时
    DWORD dwMilliseconds = config_.time_limit_ms;
    DWORD dwWaitResult = WaitForSingleObject(pi.hProcess, dwMilliseconds);
    
    if (dwWaitResult == WAIT_TIMEOUT) {
        // 超时，终止进程
        TerminateProcess(pi.hProcess, 1);
        result.timeout = true;
    }
    
    // 获取进程退出代码
    DWORD dwExitCode;
    GetExitCodeProcess(pi.hProcess, &dwExitCode);
    result.exit_code = dwExitCode;
    
    // 读取输出
    char buffer[4096];
    DWORD dwRead;
    
    // 读取标准输出
    while (ReadFile(hOutputRead, buffer, sizeof(buffer), &dwRead, NULL) && dwRead > 0) {
        result.output.append(buffer, dwRead);
    }
    
    // 读取标准错误
    while (ReadFile(hErrorRead, buffer, sizeof(buffer), &dwRead, NULL) && dwRead > 0) {
        result.error.append(buffer, dwRead);
    }
    
    // 清理
    CloseHandle(hOutputRead);
    CloseHandle(hErrorRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    return result;
}