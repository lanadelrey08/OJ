/**
 * @file discussion_handler.h
 * @brief 讨论区HTTP处理器
 * @author OJ Team
 * @date 2026-04-20
 */
#ifndef DISCUSSION_HANDLER_H
#define DISCUSSION_HANDLER_H

#include "crow_all.h"
#include "discussion_service.h"

#include <cstdint>
#include <memory>

#include <nlohmann/json.hpp>

/**
 * @class DiscussionHandler
 * @brief 讨论区接口处理器
 */
class DiscussionHandler {
public:
    /**
     * @brief 构造函数
     */
    DiscussionHandler();

    /**
     * @brief 创建主题
     */
    crow::response handleCreateTopic(const crow::request& req);

    /**
     * @brief 查询主题列表
     */
    crow::response handleListTopics(const crow::request& req);

    /**
     * @brief 查询主题详情
     */
    crow::response handleGetTopic(const crow::request& req, int64_t topic_id);

    /**
     * @brief 创建评论
     */
    crow::response handleCreateComment(const crow::request& req, int64_t topic_id);

    /**
     * @brief 查询评论列表
     */
    crow::response handleListComments(const crow::request& req, int64_t topic_id);

    /**
     * @brief 启动服务
     */
    void startServer(uint16_t port);

private:
    static crow::response jsonResponse(int status, const nlohmann::json& body);
    static size_t parsePositiveSize(const char* value, size_t fallback, size_t max_value);

    bool validateTopicPayload(const nlohmann::json& body) const;
    bool validateCommentPayload(const nlohmann::json& body) const;

    DiscussionService service_;
    std::unique_ptr<crow::Crow<crow::CORSHandler>> app_;
};

#endif  // DISCUSSION_HANDLER_H
