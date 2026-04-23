#include "discussion_service.h"

#include <cassert>
#include <optional>
#include <vector>

int main() {
    DiscussionService service;

    const int64_t topic_id = service.createTopic(1001, 42, "Two Sum question", "Can someone explain O(n) solution?");
    assert(topic_id == 1);

    const auto topics = service.listTopics(std::nullopt, 20, 0);
    assert(topics.size() == 1);
    assert(topics[0].id == topic_id);
    assert(topics[0].comment_count == 0);

    const int64_t c1 = service.createComment(topic_id, 7, "Use hash map.", std::nullopt);
    const int64_t c2 = service.createComment(topic_id, 8, "Agree with above.", c1);

    (void)c2;

    const auto comments = service.listComments(topic_id);
    assert(comments.size() == 2);
    assert(comments[0].id == c1);
    assert(comments[1].parent_comment_id == c1);

    const auto topic = service.getTopic(topic_id);
    assert(topic.has_value());
    assert(topic->comment_count == 2);

    std::vector<DiscussionTopic> no_topics = service.listTopics(9999, 20, 0);
    assert(no_topics.empty());

    return 0;
}
