CREATE TABLE IF NOT EXISTS users (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '用户ID',
    username VARCHAR(50) NOT NULL COMMENT '用户名',
    email VARCHAR(100) NOT NULL COMMENT '邮箱',
    password_hash VARCHAR(255) NOT NULL COMMENT '密码哈希',
    role ENUM('user', 'admin') DEFAULT 'user' COMMENT '角色',
    avatar_url VARCHAR(255) DEFAULT NULL COMMENT '头像URL',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    UNIQUE KEY uk_username (username),
    UNIQUE KEY uk_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

CREATE TABLE IF NOT EXISTS problems (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '题目ID',
    title VARCHAR(255) NOT NULL COMMENT '题目标题',
    description TEXT NOT NULL COMMENT '题目描述',
    difficulty ENUM('easy', 'medium', 'hard') NOT NULL COMMENT '难度',
    time_limit INT DEFAULT 1000 COMMENT '时间限制(ms)',
    memory_limit INT DEFAULT 128 COMMENT '内存限制(MB)',
    input_description TEXT NOT NULL COMMENT '输入说明',
    output_description TEXT NOT NULL COMMENT '输出说明',
    sample_input TEXT NOT NULL COMMENT '样例输入',
    sample_output TEXT NOT NULL COMMENT '样例输出',
    created_by BIGINT UNSIGNED DEFAULT NULL COMMENT '创建者ID',
    is_public BOOLEAN DEFAULT TRUE COMMENT '是否公开',
    submission_count INT DEFAULT 0 COMMENT '提交次数',
    accepted_count INT DEFAULT 0 COMMENT '通过次数',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_difficulty (difficulty),
    INDEX idx_created_by (created_by),
    INDEX idx_is_public (is_public),
    INDEX idx_created_at (created_at),
    FULLTEXT INDEX ft_title (title),
    FULLTEXT INDEX ft_description (description),
    CONSTRAINT fk_problem_creator FOREIGN KEY (created_by) REFERENCES users(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='题目表';

CREATE TABLE IF NOT EXISTS test_cases (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '测试用例ID',
    problem_id BIGINT UNSIGNED NOT NULL COMMENT '题目ID',
    input TEXT NOT NULL COMMENT '输入数据',
    output TEXT NOT NULL COMMENT '期望输出',
    is_sample BOOLEAN DEFAULT FALSE COMMENT '是否为样例',
    score INT DEFAULT 0 COMMENT '分值',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX idx_problem_id (problem_id),
    INDEX idx_is_sample (is_sample),
    CONSTRAINT fk_test_case_problem FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='测试用例表';

CREATE TABLE IF NOT EXISTS tags (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '标签ID',
    name VARCHAR(50) NOT NULL COMMENT '标签名称',
    color VARCHAR(7) DEFAULT '#1890ff' COMMENT '标签颜色',
    problem_count INT DEFAULT 0 COMMENT '关联题目数量',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    UNIQUE KEY uk_name (name),
    INDEX idx_problem_count (problem_count)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='标签表';

CREATE TABLE IF NOT EXISTS problem_tags (
    problem_id BIGINT UNSIGNED NOT NULL COMMENT '题目ID',
    tag_id BIGINT UNSIGNED NOT NULL COMMENT '标签ID',
    PRIMARY KEY (problem_id, tag_id),
    CONSTRAINT fk_problem_tags_problem FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE,
    CONSTRAINT fk_problem_tags_tag FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='题目标签关联表';
