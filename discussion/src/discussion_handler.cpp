#include "discussion_handler.h"

#include "oj/json_error.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

#include <limits>
#include <string>

using json = nlohmann::json;

DiscussionHandler::DiscussionHandler() {
    app_ = std::make_unique<crow::Crow<crow::CORSHandler>>();

    CROW_ROUTE((*app_), "/api/discussions/topics").methods("POST"_method)([this](const crow::request& req) {
        return handleCreateTopic(req);
    });

    CROW_ROUTE((*app_), "/api/discussions/topics").methods("GET"_method)([this](const crow::request& req) {
        return handleListTopics(req);
    });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>").methods("GET"_method)(
        [this](const crow::request& req, int topic_id) { return handleGetTopic(req, topic_id); });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments").methods("POST"_method)(
        [this](const crow::request& req, int topic_id) { return handleCreateComment(req, topic_id); });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments").methods("GET"_method)(
        [this](const crow::request& req, int topic_id) { return handleListComments(req, topic_id); });

    CROW_ROUTE((*app_), "/health").methods("GET"_method)([] {
        json body;
        body["status"] = "ok";
        body["service"] = "oj-discussion";
        body["mysql_pool_ready"] = oj::MysqlConnectionPool::instance().available();
        body["redis_ready"] = oj::RedisCache::instance().connected();
        auto st = oj::MysqlConnectionPool::instance().stats();
        body["mysql_idle_connections"] = st.pool_size;
        body["mysql_in_use_connections"] = st.in_use;
        return jsonResponse(200, body);
    });
}

crow::response DiscussionHandler::jsonResponse(int status, const nlohmann::json& body) {
    crow::response response(status, body.dump());
    response.add_header("Content-Type", "application/json");
    return response;
}

size_t DiscussionHandler::parsePositiveSize(const char* value, size_t fallback, size_t max_value) {
    if (!value) {
        return fallback;
    }

    try {
        unsigned long parsed = std::stoul(value);
        if (parsed == 0) {
            return fallback;
        }
        return static_cast<size_t>(std::min<unsigned long>(parsed, max_value));
    } catch (...) {
        return fallback;
    }
}

bool DiscussionHandler::validateTopicPayload(const nlohmann::json& body) const {
    return body.contains("problem_id") && body.contains("user_id") && body.contains("title") && body.contains("content");
}

bool DiscussionHandler::validateCommentPayload(const nlohmann::json& body) const {
    return body.contains("user_id") && body.contains("content");
}

crow::response DiscussionHandler::handleCreateTopic(const crow::request& req) {
    try {
        json body = json::parse(req.body);
        if (!validateTopicPayload(body)) {
            throw oj::HttpException(400, "bad_request", "missing required field in topic payload");
        }

        const std::string title = body["title"].get<std::string>();
        const std::string content = body["content"].get<std::string>();
        if (title.size() > 200) {
            throw oj::HttpException(400, "bad_request", "title too long, max length is 200");
        }
        if (content.size() > 5000) {
            throw oj::HttpException(400, "bad_request", "content too long, max length is 5000");
        }

        const int64_t topic_id = service_.createTopic(
            body["problem_id"].get<int64_t>(),
            body["user_id"].get<int64_t>(),
            title,
            content);

        auto topic = service_.getTopic(topic_id);
        OJ_LOG_INFO("discussion topic created, topic_id=" + std::to_string(topic_id));

        json result;
        result["id"] = topic_id;
        result["problem_id"] = topic->problem_id;
        result["user_id"] = topic->user_id;
        result["title"] = topic->title;
        result["content"] = topic->content;
        result["comment_count"] = topic->comment_count;
        result["created_at"] = topic->created_at;
        result["updated_at"] = topic->updated_at;
        return jsonResponse(201, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::exception& e) {
        OJ_LOG_ERROR(std::string("handleCreateTopic failed: ") + e.what());
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    }
}

crow::response DiscussionHandler::handleListTopics(const crow::request& req) {
    try {
        std::optional<int64_t> problem_id;
        const char* problem_id_param = req.url_params.get("problem_id");
        if (problem_id_param) {
            problem_id = std::stoll(problem_id_param);
        }

        const size_t limit = parsePositiveSize(req.url_params.get("limit"), 20, 200);
        const size_t offset = parsePositiveSize(req.url_params.get("offset"), 0, 50000);
        const auto topics = service_.listTopics(problem_id, limit, offset);

        json result = json::array();
        for (const auto& topic : topics) {
            json item;
            item["id"] = topic.id;
            item["problem_id"] = topic.problem_id;
            item["user_id"] = topic.user_id;
            item["title"] = topic.title;
            item["content"] = topic.content;
            item["comment_count"] = topic.comment_count;
            item["created_at"] = topic.created_at;
            item["updated_at"] = topic.updated_at;
            result.push_back(item);
        }

        return jsonResponse(200, result);
    } catch (const std::exception& e) {
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    }
}

crow::response DiscussionHandler::handleGetTopic(const crow::request& req, int64_t topic_id) {
    (void)req;
    try {
        auto topic = service_.getTopic(topic_id);
        if (!topic.has_value()) {
            throw oj::HttpException(404, "topic_not_found", "topic does not exist");
        }

        json result;
        result["id"] = topic->id;
        result["problem_id"] = topic->problem_id;
        result["user_id"] = topic->user_id;
        result["title"] = topic->title;
        result["content"] = topic->content;
        result["comment_count"] = topic->comment_count;
        result["created_at"] = topic->created_at;
        result["updated_at"] = topic->updated_at;
        return jsonResponse(200, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::exception& e) {
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleCreateComment(const crow::request& req, int64_t topic_id) {
    try {
        json body = json::parse(req.body);
        if (!validateCommentPayload(body)) {
            throw oj::HttpException(400, "bad_request", "missing required field in comment payload");
        }

        const std::string content = body["content"].get<std::string>();
        if (content.size() > 3000) {
            throw oj::HttpException(400, "bad_request", "content too long, max length is 3000");
        }

        std::optional<int64_t> parent_comment_id;
        if (body.contains("parent_comment_id") && !body["parent_comment_id"].is_null()) {
            parent_comment_id = body["parent_comment_id"].get<int64_t>();
        }

        const int64_t comment_id = service_.createComment(
            topic_id,
            body["user_id"].get<int64_t>(),
            content,
            parent_comment_id);

        OJ_LOG_INFO("discussion comment created, topic_id=" + std::to_string(topic_id) +
                    " comment_id=" + std::to_string(comment_id));

        json result;
        result["id"] = comment_id;
        result["topic_id"] = topic_id;
        result["user_id"] = body["user_id"].get<int64_t>();
        result["parent_comment_id"] = parent_comment_id.has_value() ? *parent_comment_id : 0;
        result["content"] = content;
        return jsonResponse(201, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::out_of_range&) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("topic_not_found", "topic does not exist")));
    } catch (const std::invalid_argument& e) {
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    } catch (const std::exception& e) {
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleListComments(const crow::request& req, int64_t topic_id) {
    (void)req;
    try {
        const auto comments = service_.listComments(topic_id);

        json result = json::array();
        for (const auto& comment : comments) {
            json item;
            item["id"] = comment.id;
            item["topic_id"] = comment.topic_id;
            item["user_id"] = comment.user_id;
            item["parent_comment_id"] = comment.parent_comment_id;
            item["content"] = comment.content;
            item["created_at"] = comment.created_at;
            result.push_back(item);
        }

        return jsonResponse(200, result);
    } catch (const std::out_of_range&) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("topic_not_found", "topic does not exist")));
    } catch (const std::exception& e) {
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

void DiscussionHandler::startServer(uint16_t port) {
    app_->get_middleware<crow::CORSHandler>()
        .global()
        .origin("*")
        .methods("GET"_method,
                 "POST"_method,
                 "PUT"_method,
                 "DELETE"_method,
                 "OPTIONS"_method);

    OJ_LOG_INFO("discussion HTTP server starting, port=" + std::to_string(port));
    app_->port(port).multithreaded().run();
}
