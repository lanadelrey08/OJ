#include "../include/result_judger.h"
#include <iostream>
#include <vector>

void test_trim() {
    ResultJudger judger;
    
    // 测试用例1：空字符串
    std::string test1 = "";
    std::string result1 = judger.trim(test1);
    std::cout << "Test 1 - Empty string: " << (result1 == "" ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例2：只包含空白字符的字符串
    std::string test2 = "   \t\n\r  ";
    std::string result2 = judger.trim(test2);
    std::cout << "Test 2 - Whitespace only: " << (result2 == "" ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例3：左侧有空白字符
    std::string test3 = "   hello";
    std::string result3 = judger.trim(test3);
    std::cout << "Test 3 - Leading whitespace: " << (result3 == "hello" ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例4：右侧有空白字符
    std::string test4 = "hello   ";
    std::string result4 = judger.trim(test4);
    std::cout << "Test 4 - Trailing whitespace: " << (result4 == "hello" ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例5：两侧都有空白字符
    std::string test5 = "   hello   ";
    std::string result5 = judger.trim(test5);
    std::cout << "Test 5 - Both leading and trailing whitespace: " << (result5 == "hello" ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例6：没有空白字符
    std::string test6 = "hello";
    std::string result6 = judger.trim(test6);
    std::cout << "Test 6 - No whitespace: " << (result6 == "hello" ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例7：包含制表符和换行符
    std::string test7 = "\t\n  hello  \r\n";
    std::string result7 = judger.trim(test7);
    std::cout << "Test 7 - Tabs and newlines: " << (result7 == "hello" ? "PASS" : "FAIL") << std::endl;
}

void test_compare_output() {
    ResultJudger judger;
    
    // 测试用例1：完全相同的输出
    std::string actual1 = "hello world";
    std::string expected1 = "hello world";
    bool result1 = judger.compareOutput(actual1, expected1);
    std::cout << "Test 1 - Exact match: " << (result1 ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例2：空白字符不同但内容相同
    std::string actual2 = "   hello world   ";
    std::string expected2 = "hello world";
    bool result2 = judger.compareOutput(actual2, expected2);
    std::cout << "Test 2 - Whitespace differences: " << (result2 ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例3：内容不同
    std::string actual3 = "hello";
    std::string expected3 = "world";
    bool result3 = judger.compareOutput(actual3, expected3);
    std::cout << "Test 3 - Different content: " << (!result3 ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例4：空输出
    std::string actual4 = "";
    std::string expected4 = "";
    bool result4 = judger.compareOutput(actual4, expected4);
    std::cout << "Test 4 - Empty output: " << (result4 ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例5：包含换行符的输出
    std::string actual5 = "hello\nworld";
    std::string expected5 = "hello\nworld";
    bool result5 = judger.compareOutput(actual5, expected5);
    std::cout << "Test 5 - Newlines: " << (result5 ? "PASS" : "FAIL") << std::endl;
}

void test_determine_status() {
    ResultJudger judger;
    
    // 测试用例1：超时
    SandboxResult result1;
    result1.timeout = true;
    result1.memory_exceeded = false;
    result1.exit_code = 0;
    JudgeStatus status1 = judger.determineStatus(result1, true);
    std::cout << "Test 1 - Timeout: " << (status1 == JudgeStatus::TIME_LIMIT_EXCEEDED ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例2：内存超限
    SandboxResult result2;
    result2.timeout = false;
    result2.memory_exceeded = true;
    result2.exit_code = 0;
    JudgeStatus status2 = judger.determineStatus(result2, true);
    std::cout << "Test 2 - Memory exceeded: " << (status2 == JudgeStatus::MEMORY_LIMIT_EXCEEDED ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例3：运行时错误
    SandboxResult result3;
    result3.timeout = false;
    result3.memory_exceeded = false;
    result3.exit_code = 1;
    JudgeStatus status3 = judger.determineStatus(result3, true);
    std::cout << "Test 3 - Runtime error: " << (status3 == JudgeStatus::RUNTIME_ERROR ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例4：答案错误
    SandboxResult result4;
    result4.timeout = false;
    result4.memory_exceeded = false;
    result4.exit_code = 0;
    JudgeStatus status4 = judger.determineStatus(result4, false);
    std::cout << "Test 4 - Wrong answer: " << (status4 == JudgeStatus::WRONG_ANSWER ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例5：通过
    SandboxResult result5;
    result5.timeout = false;
    result5.memory_exceeded = false;
    result5.exit_code = 0;
    JudgeStatus status5 = judger.determineStatus(result5, true);
    std::cout << "Test 5 - Accepted: " << (status5 == JudgeStatus::ACCEPTED ? "PASS" : "FAIL") << std::endl;
}

void test_judge() {
    ResultJudger judger;
    
    // 测试用例1：通过
    std::string actual1 = "42";
    std::string expected1 = "42";
    SandboxResult sandbox_result1;
    sandbox_result1.timeout = false;
    sandbox_result1.memory_exceeded = false;
    sandbox_result1.exit_code = 0;
    sandbox_result1.runtime_ms = 100;
    sandbox_result1.memory_kb = 1024;
    sandbox_result1.output = actual1;
    sandbox_result1.error = "";
    
    JudgeResult result1 = judger.judge(actual1, expected1, sandbox_result1);
    std::cout << "Test 1 - Accepted: " << (result1.overall_status == JudgeStatus::ACCEPTED ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例2：答案错误
    std::string actual2 = "43";
    std::string expected2 = "42";
    SandboxResult sandbox_result2 = sandbox_result1;
    sandbox_result2.output = actual2;
    
    JudgeResult result2 = judger.judge(actual2, expected2, sandbox_result2);
    std::cout << "Test 2 - Wrong answer: " << (result2.overall_status == JudgeStatus::WRONG_ANSWER ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例3：运行时错误
    SandboxResult sandbox_result3 = sandbox_result1;
    sandbox_result3.exit_code = 1;
    sandbox_result3.error = "Segmentation fault";
    
    JudgeResult result3 = judger.judge(actual1, expected1, sandbox_result3);
    std::cout << "Test 3 - Runtime error: " << (result3.overall_status == JudgeStatus::RUNTIME_ERROR ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例4：超时
    SandboxResult sandbox_result4 = sandbox_result1;
    sandbox_result4.timeout = true;
    
    JudgeResult result4 = judger.judge(actual1, expected1, sandbox_result4);
    std::cout << "Test 4 - Timeout: " << (result4.overall_status == JudgeStatus::TIME_LIMIT_EXCEEDED ? "PASS" : "FAIL") << std::endl;
    
    // 测试用例5：内存超限
    SandboxResult sandbox_result5 = sandbox_result1;
    sandbox_result5.memory_exceeded = true;
    
    JudgeResult result5 = judger.judge(actual1, expected1, sandbox_result5);
    std::cout << "Test 5 - Memory exceeded: " << (result5.overall_status == JudgeStatus::MEMORY_LIMIT_EXCEEDED ? "PASS" : "FAIL") << std::endl;
}

int main() {
    std::cout << "Testing ResultJudger..." << std::endl;
    std::cout << "====================================" << std::endl;
    
    std::cout << "\nTesting trim function:" << std::endl;
    test_trim();
    
    std::cout << "\nTesting compareOutput function:" << std::endl;
    test_compare_output();
    
    std::cout << "\nTesting determineStatus function:" << std::endl;
    test_determine_status();
    
    std::cout << "\nTesting judge function:" << std::endl;
    test_judge();
    
    std::cout << "\n====================================" << std::endl;
    std::cout << "All tests completed!" << std::endl;
    
    return 0;
}