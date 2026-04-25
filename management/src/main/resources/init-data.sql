USE oj;

INSERT INTO users (username, email, password_hash, role)
VALUES ('admin', 'admin@example.com', 'test_password_hash', 'admin');

INSERT INTO problems (
    title,
    description,
    difficulty,
    time_limit,
    memory_limit,
    input_description,
    output_description,
    sample_input,
    sample_output,
    created_by,
    is_public,
    submission_count,
    accepted_count
) VALUES (
    'Hello Problem',
    'This is a test problem for the problem management module.',
    'easy',
    1000,
    128,
    'Input a string.',
    'Output the same string.',
    'Hello',
    'Hello',
    1,
    TRUE,
    10,
    8
);

INSERT INTO test_cases (problem_id, input, output, is_sample, score)
VALUES
    (1, 'Hello', 'Hello', TRUE, 20),
    (1, 'World', 'World', FALSE, 80);

INSERT INTO tags (name, color, problem_count)
VALUES ('入门', '#1890ff', 1);

INSERT INTO problem_tags (problem_id, tag_id)
VALUES (1, 1);
