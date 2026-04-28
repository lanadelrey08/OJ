/**
 * @file compiler.cpp
 * @brief 编译器实现
 * @author OJ Team
 * @date 2024-01-01
 */
#include "compiler.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <sstream>

/**
 * @brief 执行命令并获取输出
 * @param command 命令字符串
 * @return 执行结果
 */
CompileResult executeCommand(const std::string& command) {
    CompileResult result;
    result.success = false;
    result.exit_code = -1;
    
    // 在Windows环境下，通过重定向标准错误到标准输出
    std::string cmd = command + " 2>&1";
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) {
        result.error = "Failed to open pipe";
        result.output = "Command: " + cmd;
        return result;
    }
    
    char buffer[1024];
    std::string output;
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }
    
    result.exit_code = _pclose(pipe);
    result.success = (result.exit_code == 0);
    result.output = "Command: " + cmd + "\nOutput: " + output;
    
    return result;
}

/**
 * @brief 保存代码到临时文件
 * @param code 代码内容
 * @param extension 文件扩展名
 * @return 临时文件路径
 */
std::string saveToTempFile(const std::string& code, const std::string& extension) {
    // 使用更可靠的临时文件生成方法
    char temp_name[L_tmpnam_s];
    errno_t err = tmpnam_s(temp_name, L_tmpnam_s);
    if (err != 0) {
        // 如果tmpnam_s失败，使用固定的临时文件名
        static int counter = 0;
        std::string temp_file = "temp_" + std::to_string(counter++) + extension;
        std::ofstream file(temp_file);
        if (file.is_open()) {
            file << code;
            file.close();
        }
        return temp_file;
    }
    
    std::string temp_file = std::string(temp_name) + extension;
    std::ofstream file(temp_file);
    if (file.is_open()) {
        file << code;
        file.close();
    }
    
    return temp_file;
}

/**
 * @brief 编译C代码
 * @param source_code C源代码
 * @param output_path 输出文件路径
 * @return 编译结果
 */
CompileResult CCompiler::compile(const std::string& source_code, const std::string& output_path) {
    std::string temp_file = saveToTempFile(source_code, ".c");
    std::string command = "gcc -std=c11 -O2 -o " + output_path + " " + temp_file;
    
    CompileResult result = executeCommand(command);
    
    // 清理临时文件
    remove(temp_file.c_str());
    
    return result;
}

/**
 * @brief 编译C++代码
 * @param source_code C++源代码
 * @param output_path 输出文件路径
 * @return 编译结果
 */
CompileResult CppCompiler::compile(const std::string& source_code, const std::string& output_path) {
    std::string temp_file = saveToTempFile(source_code, ".cpp");
    std::string command = "g++ -std=c++17 -O2 -o " + output_path + " " + temp_file;
    
    CompileResult result = executeCommand(command);
    
    // 清理临时文件
    remove(temp_file.c_str());
    
    return result;
}

/**
 * @brief 编译Java代码
 * @param source_code Java源代码
 * @param output_path 输出文件路径
 * @return 编译结果
 */
CompileResult JavaCompiler::compile(const std::string& source_code, const std::string& output_path) {
    // 对于Java，需要确保文件名与公共类名相同
    std::string temp_file = output_path;
    
    // 保存代码到文件
    std::ofstream file(temp_file);
    if (file.is_open()) {
        file << source_code;
        file.close();
    }
    
    std::string command = "javac " + temp_file;
    CompileResult result = executeCommand(command);
    
    return result;
}

/**
 * @brief 编译Python代码
 * @param source_code Python源代码
 * @param output_path 输出文件路径
 * @return 编译结果
 */
CompileResult PythonCompiler::compile(const std::string& source_code, const std::string& output_path) {
    // Python 是解释型语言，不需要编译
    CompileResult result;
    result.success = true;
    result.exit_code = 0;
    result.output = "Python code does not require compilation";
    
    // 保存代码到文件
    std::ofstream file(output_path);
    if (file.is_open()) {
        file << source_code;
        file.close();
    }
    
    return result;
}