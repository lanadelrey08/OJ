#include "discussion_service.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string DiscussionService::nowTimeString() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

int64_t DiscussionService::createTopic(int64_t problem_id,
                                       int64_t user_id,
                                       const std::string& title,
                                       const std::string& content) {
    if (problem_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("problem_id and user_id must be positive");
    }
    if (title.empty() || content.empty()) {
        throw std::invalid_argument("title/content cannot be empty");
    }

    std::lock_guard<std::mutex> lock(mutex_);

    DiscussionTopic topic;
    topic.id = next_topic_id_++;
    topic.problem_id = problem_id;
    topic.user_id = user_id;
    topic.title = title;
    topic.content = content;
    topic.created_at = nowTimeString();
    topic.updated_at = topic.created_at;

    topics_[topic.id] = topic;
    return topic.id;
}

std::vector<DiscussionTopic> DiscussionService::listTopics(std::optional<int64_t> problem_id,
                                                           size_t limit,
                                                           size_t offset) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<DiscussionTopic> output;
    output.reserve(topics_.size());

    for (const auto& item : topics_) {
        const DiscussionTopic& topic = item.second;
        if (problem_id.has_value() && topic.problem_id != *problem_id) {
            continue;
        }
        output.push_back(topic);
    }

    std::sort(output.begin(), output.end(), [](const DiscussionTopic& lhs, const DiscussionTopic& rhs) {
        return lhs.id > rhs.id;
    });

    if (offset >= output.size()) {
        return {};
    }

    const size_t end = std::min(output.size(), offset + limit);
    return std::vector<DiscussionTopic>(output.begin() + static_cast<std::ptrdiff_t>(offset),
                                        output.begin() + static_cast<std::ptrdiff_t>(end));
}

std::optional<DiscussionTopic> DiscussionService::getTopic(int64_t topic_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = topics_.find(topic_id);
    if (it == topics_.end()) {
        return std::nullopt;
    }
    return it->second;
}

int64_t DiscussionService::createComment(int64_t topic_id,
                                         int64_t user_id,
                                         const std::string& content,
                                         std::optional<int64_t> parent_comment_id) {
    if (topic_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("topic_id and user_id must be positive");
    }
    if (content.empty()) {
        throw std::invalid_argument("content cannot be empty");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto topic_it = topics_.find(topic_id);
    if (topic_it == topics_.end()) {
        throw std::out_of_range("topic not found");
    }

    if (parent_comment_id.has_value()) {
        auto parent_it = comment_to_topic_.find(*parent_comment_id);
        if (parent_it == comment_to_topic_.end() || parent_it->second != topic_id) {
            throw std::invalid_argument("parent_comment_id is invalid");
        }
    }

    DiscussionComment comment;
    comment.id = next_comment_id_++;
    comment.topic_id = topic_id;
    comment.user_id = user_id;
    comment.parent_comment_id = parent_comment_id.has_value() ? *parent_comment_id : 0;
    comment.content = content;
    comment.created_at = nowTimeString();

    comment_to_topic_[comment.id] = topic_id;
    comments_by_topic_[topic_id].push_back(comment);

    topic_it->second.comment_count += 1;
    topic_it->second.updated_at = comment.created_at;

    return comment.id;
}

std::vector<DiscussionComment> DiscussionService::listComments(int64_t topic_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto topic_it = topics_.find(topic_id);
    if (topic_it == topics_.end()) {
        throw std::out_of_range("topic not found");
    }

    auto it = comments_by_topic_.find(topic_id);
    if (it == comments_by_topic_.end()) {
        return {};
    }

    std::vector<DiscussionComment> comments = it->second;
    std::sort(comments.begin(), comments.end(), [](const DiscussionComment& lhs, const DiscussionComment& rhs) {
        return lhs.id < rhs.id;
    });
    return comments;
}
