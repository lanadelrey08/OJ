/**
 * @file submission_handler.cpp
 * @brief 提交处理器实现
 * @author OJ Team
 * @date 2024-01-01
 */
#include "submission_handler.h"

#include "oj/json_error.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

#include <nlohmann/json.hpp>
#include <iostream>
#include <sstream>
#include <atomic>
#include <chrono>

#ifdef OJ_WITH_MYSQL
#if __has_include(<mysql_driver.h>)
#include <mysql_driver.h>
#elif __has_include(<jdbc/mysql_driver.h>)
#include <jdbc/mysql_driver.h>
#endif
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>
#endif

using json = nlohmann::json;

// 需要检查
/**
 * @brief 从数据库获取题目信息和测试用例
 * @param problem_id 题目ID
 * @param[out] time_limit_ms 时间限制（毫秒）
 * @param[out] memory_limit_mb 内存限制（MB）
 * @param[out] test_cases 测试用例列表
 * @return 是否获取成功
 */
bool getProblemFromDatabase(int problem_id, int& time_limit_ms, int& memory_limit_mb, 
                            std::vector<TestCase>& test_cases) {
    void* conn_ptr = oj::MysqlConnectionPool::instance().acquire();
    if (!conn_ptr) {
        OJ_LOG_ERROR("Failed to acquire MySQL connection");
        return false;
    }
    
    bool success = false;
    
#ifdef OJ_WITH_MYSQL
    sql::Connection* conn = static_cast<sql::Connection*>(conn_ptr);
    std::unique_ptr<sql::Statement> stmt;
    std::unique_ptr<sql::ResultSet> res;
    
    try {
        stmt.reset(conn->createStatement());
        
        std::string query = "SELECT time_limit, memory_limit FROM problems WHERE id = " + std::to_string(problem_id);
        res.reset(stmt->executeQuery(query));
        
        if (!res->next()) {
            OJ_LOG_ERROR("Problem not found: " << problem_id);
            goto cleanup;
        }
        
        time_limit_ms = res->getInt("time_limit");
        memory_limit_mb = res->getInt("memory_limit");
        
        query = "SELECT id, input, output, score FROM test_cases WHERE problem_id = " + std::to_string(problem_id) + " ORDER BY id";
        res.reset(stmt->executeQuery(query));
        
        while (res->next()) {
            TestCase test_case;
            test_case.id = res->getInt("id");
            test_case.input = res->getString("input");
            test_case.expected_output = res->getString("output");
            test_case.score = res->getInt("score");
            test_cases.push_back(test_case);
        }
        
        if (test_cases.empty()) {
            OJ_LOG_WARN("No test cases found for problem: " << problem_id);
        }
        
        success = true;
    } catch (const sql::SQLException& e) {
        OJ_LOG_ERROR("MySQL query error: " << e.what());
        success = false;
    } catch (const std::exception& e) {
        OJ_LOG_ERROR("Exception in getProblemFromDatabase: " << e.what());
        success = false;
    }
#endif
    
cleanup:
    oj::MysqlConnectionPool::instance().release(conn_ptr);
    return success;
}

/**
 * @brief 生成唯一提交ID
 * @return 提交ID
 */
int64_t generateSubmissionId() {
    static std::atomic<int64_t> counter{0};
    return std::chrono::system_clock::now().time_since_epoch().count() + counter++;
}

/**
 * @brief 构造函数
 */
SubmissionHandler::SubmissionHandler() {
    judge_engine_ = std::make_unique<JudgeEngine>();
    app_ = std::make_unique<crow::Crow<crow::CORSHandler>>();

    CROW_ROUTE((*app_), "/api/submit").methods("POST"_method)([this](const crow::request& req) {
        return handleSubmit(req);
    });

    CROW_ROUTE((*app_), "/api/submissions").methods("GET"_method)([this](const crow::request& req) {
        return handleGetSubmissions(req);
    });

    CROW_ROUTE((*app_), "/api/submissions/<int>").methods("GET"_method)(
        [this](const crow::request& req, int id) { return handleGetSubmission(req, id); });

    CROW_ROUTE((*app_), "/api/submissions/<int>/status").methods("GET"_method)(
        [this](const crow::request& req, int id) { return handleSubmissionStatus(req, id); });

    CROW_ROUTE((*app_), "/health").methods("GET"_method)([] {
        json j;
        j["status"] = "ok";
        j["service"] = "oj-backend";
        j["mysql_pool_ready"] = oj::MysqlConnectionPool::instance().available();
        j["redis_ready"] = oj::RedisCache::instance().connected();
        auto st = oj::MysqlConnectionPool::instance().stats();
        j["mysql_idle_connections"] = st.pool_size;
        j["mysql_in_use_connections"] = st.in_use;
        crow::response res(200, j.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    });
}

/**
 * @brief 验证提交
 * @param req HTTP请求
 * @return 是否验证通过
 */
bool SubmissionHandler::validateSubmission(const crow::request& req) {
    try {
        auto body = json::parse(req.body);
        if (!body.contains("code") || !body.contains("language") || !body.contains("problem_id")) {
            return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * @brief 处理代码提交
 * @param req HTTP请求
 * @return HTTP响应
 */
crow::response SubmissionHandler::handleSubmit(const crow::request& req) {
    // 验证提交格式
    if (!validateSubmission(req)) {
        crow::response r(400, oj::makeErrorJson("bad_request", "Invalid submission format"));
        r.add_header("Content-Type", "application/json");
        return r;
    }

    try {
        auto body = json::parse(req.body);
        std::string code = body["code"];
        std::string language = body["language"];
        int problem_id = body["problem_id"];
        
        OJ_LOG_INFO("submit problem_id=" + std::to_string(problem_id) + " lang=" + language);
        
        // 从数据库获取题目信息和测试用例
        std::vector<TestCase> test_cases;
        int time_limit_ms = 1000;
        int memory_limit_mb = 128;
        
        if (!getProblemFromDatabase(problem_id, time_limit_ms, memory_limit_mb, test_cases)) {
            crow::response r(500, oj::makeErrorJson("database_error", "Failed to fetch problem from database"));
            r.add_header("Content-Type", "application/json");
            return r;
        }
        if (test_cases.empty()) {
            crow::response r(404, oj::makeErrorJson("not_found", "No test cases found for problem"));
            r.add_header("Content-Type", "application/json");
            return r;
        }
        
        OJ_LOG_INFO("Loaded " + std::to_string(test_cases.size()) + " test cases for problem " + std::to_string(problem_id));
        OJ_LOG_INFO("Time limit: " + std::to_string(time_limit_ms) + "ms, Memory limit: " + std::to_string(memory_limit_mb) + "MB");
        
        // 评测代码
        JudgeResult result = judge_engine_->judge(code, language, test_cases, time_limit_ms, memory_limit_mb);

        // 构建响应
        json response_data;
        response_data["submission_id"] = generateSubmissionId();
        response_data["problem_id"] = problem_id;
        response_data["language"] = language;
        response_data["status"] = static_cast<int>(result.overall_status);
        response_data["runtime"] = result.runtime_ms;
        response_data["memory"] = result.memory_kb;
        response_data["error_message"] = result.error_message;
        
        json test_case_results;
        for (const auto& tc_result : result.test_case_results) {
            json tc_data;
            tc_data["case_id"] = tc_result.case_id;
            tc_data["status"] = static_cast<int>(tc_result.status);
            tc_data["runtime"] = tc_result.runtime_ms;
            tc_data["memory"] = tc_result.memory_kb;
            tc_data["passed"] = tc_result.passed;
            test_case_results.push_back(tc_data);
        }
        response_data["test_cases"] = test_case_results;
        
        crow::response ok(200, response_data.dump());
        ok.add_header("Content-Type", "application/json");
        return ok;
    } catch (const oj::HttpException& e) {
        crow::response r(e.http_status, oj::makeErrorJson(e.error_code, e.what()));
        r.add_header("Content-Type", "application/json");
        return r;
    } catch (const std::exception& e) {
        OJ_LOG_ERROR(std::string("handleSubmit: ") + e.what());
        crow::response r(500, oj::makeErrorJson("internal_error", e.what()));
        r.add_header("Content-Type", "application/json");
        return r;
    }
}

/**
 * @brief 获取提交列表
 * @param req HTTP请求
 * @return HTTP响应
 */
crow::response SubmissionHandler::handleGetSubmissions(const crow::request& req) {
    json response_data = json::array();
    
#ifdef OJ_WITH_MYSQL
    void* conn_ptr = oj::MysqlConnectionPool::instance().acquire();
    if (!conn_ptr) {
        OJ_LOG_ERROR("Failed to acquire MySQL connection");
        crow::response r(500, oj::makeErrorJson("database_error", "Failed to connect to database"));
        r.add_header("Content-Type", "application/json");
        return r;
    }
    
    sql::Connection* conn = static_cast<sql::Connection*>(conn_ptr);
    std::unique_ptr<sql::Statement> stmt;
    std::unique_ptr<sql::ResultSet> res;
    
    try {
        stmt.reset(conn->createStatement());
        std::string query = "SELECT id, problem_id, verdict, submit_at FROM submissions ORDER BY submit_at DESC LIMIT 100";
        res.reset(stmt->executeQuery(query));
        
        while (res->next()) {
            json submission;
            submission["id"] = res->getInt64("id");
            submission["problem_id"] = res->getInt64("problem_id");
            submission["verdict"] = res->getString("verdict");
            submission["submit_time"] = res->getInt64("submit_at");
            response_data.push_back(submission);
        }
    } catch (const sql::SQLException& e) {
        OJ_LOG_ERROR("MySQL query error: " << e.what());
        oj::MysqlConnectionPool::instance().release(conn_ptr);
        crow::response r(500, oj::makeErrorJson("database_error", "Failed to fetch submissions"));
        r.add_header("Content-Type", "application/json");
        return r;
    } catch (const std::exception& e) {
        OJ_LOG_ERROR("Exception in handleGetSubmissions: " << e.what());
        oj::MysqlConnectionPool::instance().release(conn_ptr);
        crow::response r(500, oj::makeErrorJson("internal_error", e.what()));
        r.add_header("Content-Type", "application/json");
        return r;
    }
    
    oj::MysqlConnectionPool::instance().release(conn_ptr);
#endif
    
    crow::response r(200, response_data.dump());
    r.add_header("Content-Type", "application/json");
    return r;
}

/**
 * @brief 获取提交详情
 * @param req HTTP请求
 * @param id 提交ID
 * @return HTTP响应
 */
crow::response SubmissionHandler::handleGetSubmission(const crow::request& req, int64_t id) {
    json response_data;
    
#ifdef OJ_WITH_MYSQL
    void* conn_ptr = oj::MysqlConnectionPool::instance().acquire();
    if (!conn_ptr) {
        OJ_LOG_ERROR("Failed to acquire MySQL connection");
        crow::response r(500, oj::makeErrorJson("database_error", "Failed to connect to database"));
        r.add_header("Content-Type", "application/json");
        return r;
    }
    
    sql::Connection* conn = static_cast<sql::Connection*>(conn_ptr);
    std::unique_ptr<sql::Statement> stmt;
    std::unique_ptr<sql::ResultSet> res;
    
    try {
        stmt.reset(conn->createStatement());
        std::string query = "SELECT id, problem_id, verdict, submit_at FROM submissions WHERE id = " + std::to_string(id);
        res.reset(stmt->executeQuery(query));
        
        if (!res->next()) {
            oj::MysqlConnectionPool::instance().release(conn_ptr);
            crow::response r(404, oj::makeErrorJson("not_found", "Submission not found"));
            r.add_header("Content-Type", "application/json");
            return r;
        }
        
        response_data["id"] = res->getInt64("id");
        response_data["problem_id"] = res->getInt64("problem_id");
        response_data["verdict"] = res->getString("verdict");
        response_data["submit_time"] = res->getInt64("submit_at");
        
    } catch (const sql::SQLException& e) {
        OJ_LOG_ERROR("MySQL query error: " << e.what());
        oj::MysqlConnectionPool::instance().release(conn_ptr);
        crow::response r(500, oj::makeErrorJson("database_error", "Failed to fetch submission"));
        r.add_header("Content-Type", "application/json");
        return r;
    } catch (const std::exception& e) {
        OJ_LOG_ERROR("Exception in handleGetSubmission: " << e.what());
        oj::MysqlConnectionPool::instance().release(conn_ptr);
        crow::response r(500, oj::makeErrorJson("internal_error", e.what()));
        r.add_header("Content-Type", "application/json");
        return r;
    }
    
    oj::MysqlConnectionPool::instance().release(conn_ptr);
#endif
    
    crow::response r(200, response_data.dump());
    r.add_header("Content-Type", "application/json");
    return r;
}

/**
 * @brief 获取评测状态（SSE）
 * @param req HTTP请求
 * @param id 提交ID
 * @return HTTP响应
 */
crow::response SubmissionHandler::handleSubmissionStatus(const crow::request& req, int64_t id) {
    json status_data;
    bool found = false;
    
#ifdef OJ_WITH_MYSQL
    void* conn_ptr = oj::MysqlConnectionPool::instance().acquire();
    if (!conn_ptr) {
        OJ_LOG_ERROR("Failed to acquire MySQL connection");
        crow::response r(500, oj::makeErrorJson("database_error", "Failed to connect to database"));
        r.add_header("Content-Type", "application/json");
        return r;
    }
    
    sql::Connection* conn = static_cast<sql::Connection*>(conn_ptr);
    std::unique_ptr<sql::Statement> stmt;
    std::unique_ptr<sql::ResultSet> res;
    
    try {
        stmt.reset(conn->createStatement());
        std::string query = "SELECT id, verdict, submit_at FROM submissions WHERE id = " + std::to_string(id);
        res.reset(stmt->executeQuery(query));
        
        if (res->next()) {
            status_data["id"] = res->getInt64("id");
            status_data["verdict"] = res->getString("verdict");
            status_data["submit_time"] = res->getInt64("submit_at");
            found = true;
        }
        
    } catch (const sql::SQLException& e) {
        OJ_LOG_ERROR("MySQL query error: " << e.what());
        oj::MysqlConnectionPool::instance().release(conn_ptr);
        crow::response r(500, oj::makeErrorJson("database_error", "Failed to fetch submission status"));
        r.add_header("Content-Type", "application/json");
        return r;
    } catch (const std::exception& e) {
        OJ_LOG_ERROR("Exception in handleSubmissionStatus: " << e.what());
        oj::MysqlConnectionPool::instance().release(conn_ptr);
        crow::response r(500, oj::makeErrorJson("internal_error", e.what()));
        r.add_header("Content-Type", "application/json");
        return r;
    }
    
    oj::MysqlConnectionPool::instance().release(conn_ptr);
#endif
    
    if (!found) {
        crow::response r(404, oj::makeErrorJson("not_found", "Submission not found"));
        r.add_header("Content-Type", "application/json");
        return r;
    }
    
    // 构建 SSE 响应
    std::string response = "event: status\ndata: " + status_data.dump() + "\n\n";
    crow::response r(200, response);
    r.add_header("Content-Type", "text/event-stream");
    r.add_header("Cache-Control", "no-cache");
    r.add_header("Connection", "keep-alive");
    return r;
}

/**
 * @brief 启动服务器
 * @param port 端口号
 */
void SubmissionHandler::startServer(uint16_t port) {
    app_->get_middleware<crow::CORSHandler>()
        .global()
        .origin("*")
        .methods("GET"_method,
                 "POST"_method,
                 "PUT"_method,
                 "DELETE"_method,
                 "OPTIONS"_method);

    OJ_LOG_INFO("HTTP server starting, port=" + std::to_string(port));
    app_->port(port).multithreaded().run();
}