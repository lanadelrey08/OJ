/**
 * @file discussion_service.h
 * @brief 讨论区业务服务
 * @author OJ Team
 * @date 2026-04-20
 */
#ifndef DISCUSSION_SERVICE_H
#define DISCUSSION_SERVICE_H

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @struct DiscussionTopic
 * @brief 讨论主题结构体
 */
struct DiscussionTopic {
    int64_t id = 0;                    ///< 主题ID
    int64_t problem_id = 0;            ///< 关联题目ID
    int64_t user_id = 0;               ///< 发布用户ID
    std::string title;                 ///< 标题
    std::string content;               ///< 内容
    std::string created_at;            ///< 创建时间
    std::string updated_at;            ///< 更新时间
    int comment_count = 0;             ///< 评论数量
};

/**
 * @struct DiscussionComment
 * @brief 讨论评论结构体
 */
struct DiscussionComment {
    int64_t id = 0;                    ///< 评论ID
    int64_t topic_id = 0;              ///< 所属主题ID
    int64_t user_id = 0;               ///< 评论用户ID
    int64_t parent_comment_id = 0;     ///< 父评论ID（0表示顶级评论）
    std::string content;               ///< 评论内容
    std::string created_at;            ///< 创建时间
};

/**
 * @class DiscussionService
 * @brief 讨论区业务类（线程安全）
 */
class DiscussionService {
public:
    /**
     * @brief 创建主题
     */
    int64_t createTopic(int64_t problem_id,
                        int64_t user_id,
                        const std::string& title,
                        const std::string& content);

    /**
     * @brief 查询主题列表
     */
    std::vector<DiscussionTopic> listTopics(std::optional<int64_t> problem_id,
                                            size_t limit,
                                            size_t offset) const;

    /**
     * @brief 获取主题详情
     */
    std::optional<DiscussionTopic> getTopic(int64_t topic_id) const;

    /**
     * @brief 创建评论
     */
    int64_t createComment(int64_t topic_id,
                          int64_t user_id,
                          const std::string& content,
                          std::optional<int64_t> parent_comment_id);

    /**
     * @brief 查询主题评论
     */
    std::vector<DiscussionComment> listComments(int64_t topic_id) const;

private:
    static std::string nowTimeString();

    mutable std::mutex mutex_;
    int64_t next_topic_id_ = 1;
    int64_t next_comment_id_ = 1;

    std::unordered_map<int64_t, DiscussionTopic> topics_;
    std::unordered_map<int64_t, std::vector<DiscussionComment>> comments_by_topic_;
    std::unordered_map<int64_t, int64_t> comment_to_topic_;
};

#endif  // DISCUSSION_SERVICE_H
