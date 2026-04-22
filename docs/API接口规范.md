# 在线判题系统（OJ）- API接口规范

## 一、接口概述

### 1.1 基础信息
- **协议**: HTTPS
- **数据格式**: JSON
- **字符编码**: UTF-8
- **Base URL**: `https://api.oj.example.com/v1`

### 1.2 通用响应格式

#### 成功响应
```json
{
  "code": 200,
  "message": "success",
  "data": {}
}
```

#### 错误响应
```json
{
  "code": 400,
  "message": "参数错误",
  "data": null
}
```

### 1.3 HTTP状态码

| 状态码 | 含义 | 说明 |
|--------|------|------|
| 200 | OK | 请求成功 |
| 201 | Created | 创建成功 |
| 400 | Bad Request | 请求参数错误 |
| 401 | Unauthorized | 未认证或Token过期 |
| 403 | Forbidden | 权限不足 |
| 404 | Not Found | 资源不存在 |
| 500 | Internal Server Error | 服务器内部错误 |

### 1.4 认证方式

所有需要认证的接口需在请求头中添加：
```
Authorization: Bearer <JWT_TOKEN>
```

---

## 二、用户管理接口

### 2.1 用户注册

**接口**: `POST /users/register`

**请求参数**:
```json
{
  "username": "string",    // 必填，3-20字符，字母数字下划线
  "email": "string",       // 必填，有效邮箱格式
  "password": "string"     // 必填，6-20字符
}
```

**响应示例**:
```json
{
  "code": 201,
  "message": "注册成功",
  "data": {
    "id": 1,
    "username": "john_doe",
    "email": "john@example.com",
    "created_at": "2026-03-17T10:00:00Z"
  }
}
```

**错误码**:
- 400: 参数错误（用户名已存在/邮箱已注册/密码不符合要求）

### 2.2 用户登录

**接口**: `POST /users/login`

**请求参数**:
```json
{
  "username": "string",    // 必填
  "password": "string"     // 必填
}
```

**响应示例**:
```json
{
  "code": 200,
  "message": "登录成功",
  "data": {
    "user": {
      "id": 1,
      "username": "john_doe",
      "email": "john@example.com",
      "role": "user",
      "avatar_url": "https://..."
    },
    "token": "eyJhbGciOiJIUzI1NiIs...",
    "refresh_token": "eyJhbGciOiJIUzI1NiIs...",
    "expires_in": 86400
  }
}
```

**错误码**:
- 401: 用户名或密码错误

### 2.3 用户登出

**接口**: `POST /users/logout`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**响应示例**:
```json
{
  "code": 200,
  "message": "登出成功",
  "data": null
}
```

### 2.4 获取当前用户信息

**接口**: `GET /users/me`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "id": 1,
    "username": "john_doe",
    "email": "john@example.com",
    "role": "user",
    "avatar_url": "https://...",
    "solved_count": 50,
    "submission_count": 100,
    "created_at": "2026-03-17T10:00:00Z"
  }
}
```

### 2.5 更新用户信息

**接口**: `PUT /users/me`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**请求参数**:
```json
{
  "email": "string",       // 可选
  "avatar_url": "string"   // 可选
}
```

**响应示例**:
```json
{
  "code": 200,
  "message": "更新成功",
  "data": {
    "id": 1,
    "username": "john_doe",
    "email": "new_email@example.com",
    "avatar_url": "https://...",
    "updated_at": "2026-03-17T12:00:00Z"
  }
}
```

### 2.6 修改密码

**接口**: `PUT /users/me/password`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**请求参数**:
```json
{
  "old_password": "string",    // 必填
  "new_password": "string"     // 必填，6-20字符
}
```

**响应示例**:
```json
{
  "code": 200,
  "message": "密码修改成功",
  "data": null
}
```

---

## 三、题目管理接口

### 3.1 获取题目列表

**接口**: `GET /problems`

**查询参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| page | int | 否 | 页码，默认1 |
| size | int | 否 | 每页数量，默认20，最大100 |
| difficulty | string | 否 | 难度筛选：easy/medium/hard |
| tag | string | 否 | 标签筛选 |
| keyword | string | 否 | 关键词搜索 |

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "total": 100,
    "page": 1,
    "size": 20,
    "problems": [
      {
        "id": 1,
        "title": "两数之和",
        "difficulty": "easy",
        "tags": ["数组", "哈希表"],
        "pass_rate": 0.65,
        "submission_count": 1000,
        "accepted_count": 650
      }
    ]
  }
}
```

### 3.2 获取题目详情

**接口**: `GET /problems/{id}`

**路径参数**:
- id: 题目ID

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "id": 1,
    "title": "两数之和",
    "description": "给定一个整数数组 nums 和一个整数目标值 target...",
    "difficulty": "easy",
    "time_limit": 1000,
    "memory_limit": 128,
    "input_description": "输入包含多组测试数据...",
    "output_description": "输出每个测试数据的结果...",
    "sample_input": "4 9\n2 7 11 15",
    "sample_output": "0 1",
    "tags": ["数组", "哈希表"],
    "created_at": "2026-03-17T10:00:00Z",
    "updated_at": "2026-03-17T10:00:00Z"
  }
}
```

### 3.3 创建题目（管理员）

**接口**: `POST /problems`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**请求参数**:
```json
{
  "title": "string",              // 必填
  "description": "string",        // 必填
  "difficulty": "string",         // 必填：easy/medium/hard
  "time_limit": 1000,             // 可选，默认1000ms
  "memory_limit": 128,            // 可选，默认128MB
  "input_description": "string",  // 必填
  "output_description": "string", // 必填
  "sample_input": "string",       // 必填
  "sample_output": "string",      // 必填
  "tags": ["string"],             // 可选
  "test_cases": [                 // 必填
    {
      "input": "string",
      "output": "string",
      "is_sample": true
    }
  ]
}
```

**响应示例**:
```json
{
  "code": 201,
  "message": "创建成功",
  "data": {
    "id": 2,
    "title": "新题目",
    "created_at": "2026-03-17T10:00:00Z"
  }
}
```

### 3.4 更新题目（管理员）

**接口**: `PUT /problems/{id}`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**请求参数**: 同创建题目

**响应示例**:
```json
{
  "code": 200,
  "message": "更新成功",
  "data": {
    "id": 2,
    "title": "更新的题目",
    "updated_at": "2026-03-17T12:00:00Z"
  }
}
```

### 3.5 删除题目（管理员）

**接口**: `DELETE /problems/{id}`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**响应示例**:
```json
{
  "code": 200,
  "message": "删除成功",
  "data": null
}
```

---

## 四、代码评测接口

### 4.1 提交代码

**接口**: `POST /submissions`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**请求参数**:
```json
{
  "problem_id": 1,        // 必填
  "code": "string",       // 必填
  "language": "cpp"       // 必填：c/cpp/java/python
}
```

**响应示例**:
```json
{
  "code": 201,
  "message": "提交成功",
  "data": {
    "id": 100,
    "problem_id": 1,
    "status": "pending",
    "submitted_at": "2026-03-17T10:00:00Z"
  }
}
```

### 4.2 获取提交记录

**接口**: `GET /submissions`

**查询参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| page | int | 否 | 页码，默认1 |
| size | int | 否 | 每页数量，默认20 |
| problem_id | int | 否 | 题目ID筛选 |
| status | string | 否 | 状态筛选 |
| language | string | 否 | 语言筛选 |

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "total": 100,
    "page": 1,
    "size": 20,
    "submissions": [
      {
        "id": 100,
        "problem_id": 1,
        "problem_title": "两数之和",
        "language": "cpp",
        "status": "accepted",
        "runtime_ms": 12,
        "memory_kb": 10240,
        "submitted_at": "2026-03-17T10:00:00Z"
      }
    ]
  }
}
```

### 4.3 获取提交详情

**接口**: `GET /submissions/{id}`

**路径参数**:
- id: 提交记录ID

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "id": 100,
    "problem_id": 1,
    "problem_title": "两数之和",
    "user_id": 1,
    "username": "john_doe",
    "code": "#include <iostream>...",
    "language": "cpp",
    "status": "accepted",
    "runtime_ms": 12,
    "memory_kb": 10240,
    "error_message": null,
    "test_case_results": [
      {
        "case_id": 1,
        "status": "passed",
        "runtime_ms": 10,
        "memory_kb": 10240
      }
    ],
    "submitted_at": "2026-03-17T10:00:00Z"
  }
}
```

### 4.4 获取评测状态（WebSocket/SSE）

**接口**: `GET /submissions/{id}/status` (SSE)

**路径参数**:
- id: 提交记录ID

**响应示例** (SSE事件流):
```
event: status
data: {"status": "judging", "progress": 50}

event: status
data: {"status": "accepted", "runtime_ms": 12, "memory_kb": 10240}
```

---

## 五、排行榜接口

### 5.1 获取总排行榜

**接口**: `GET /rankings`

**查询参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| page | int | 否 | 页码，默认1 |
| size | int | 否 | 每页数量，默认50 |

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "total": 1000,
    "page": 1,
    "size": 50,
    "rankings": [
      {
        "rank": 1,
        "user_id": 1,
        "username": "john_doe",
        "avatar_url": "https://...",
        "solved_count": 100,
        "submission_count": 150,
        "acceptance_rate": 0.67
      }
    ]
  }
}
```

### 5.2 获取用户排名

**接口**: `GET /rankings/me`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "rank": 50,
    "solved_count": 50,
    "submission_count": 100,
    "acceptance_rate": 0.50
  }
}
```

---

## 六、讨论区接口

### 6.1 获取讨论列表

**接口**: `GET /discussions`

**查询参数**:
| 参数 | 类型 | 必填 | 说明 |
|------|------|------|------|
| page | int | 否 | 页码，默认1 |
| size | int | 否 | 每页数量，默认20 |
| problem_id | int | 否 | 题目ID筛选 |
| keyword | string | 否 | 关键词搜索 |

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "total": 100,
    "page": 1,
    "size": 20,
    "discussions": [
      {
        "id": 1,
        "title": "这道题的解法讨论",
        "problem_id": 1,
        "problem_title": "两数之和",
        "author_id": 1,
        "author_name": "john_doe",
        "reply_count": 10,
        "view_count": 100,
        "is_pinned": false,
        "created_at": "2026-03-17T10:00:00Z"
      }
    ]
  }
}
```

### 6.2 创建讨论

**接口**: `POST /discussions`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**请求参数**:
```json
{
  "title": "string",      // 必填
  "content": "string",    // 必填
  "problem_id": 1         // 可选，关联题目
}
```

**响应示例**:
```json
{
  "code": 201,
  "message": "创建成功",
  "data": {
    "id": 1,
    "title": "这道题的解法讨论",
    "created_at": "2026-03-17T10:00:00Z"
  }
}
```

### 6.3 获取讨论详情

**接口**: `GET /discussions/{id}`

**路径参数**:
- id: 讨论ID

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "id": 1,
    "title": "这道题的解法讨论",
    "content": "我认为这道题可以用哈希表来解决...",
    "problem_id": 1,
    "problem_title": "两数之和",
    "author_id": 1,
    "author_name": "john_doe",
    "view_count": 100,
    "is_pinned": false,
    "created_at": "2026-03-17T10:00:00Z",
    "replies": [
      {
        "id": 1,
        "content": "同意，哈希表的时间复杂度是O(n)",
        "author_id": 2,
        "author_name": "jane_doe",
        "created_at": "2026-03-17T11:00:00Z"
      }
    ]
  }
}
```

### 6.4 回复讨论

**接口**: `POST /discussions/{id}/replies`

**请求头**:
```
Authorization: Bearer <JWT_TOKEN>
```

**请求参数**:
```json
{
  "content": "string"    // 必填
}
```

**响应示例**:
```json
{
  "code": 201,
  "message": "回复成功",
  "data": {
    "id": 2,
    "content": "谢谢分享！",
    "created_at": "2026-03-17T12:00:00Z"
  }
}
```

---

## 七、系统接口

### 7.1 健康检查

**接口**: `GET /health`

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "status": "healthy",
    "version": "1.0.0",
    "timestamp": "2026-03-17T10:00:00Z"
  }
}
```

### 7.2 获取支持的语言

**接口**: `GET /languages`

**响应示例**:
```json
{
  "code": 200,
  "message": "success",
  "data": {
    "languages": [
      {
        "id": "c",
        "name": "C",
        "version": "C11"
      },
      {
        "id": "cpp",
        "name": "C++",
        "version": "C++17"
      },
      {
        "id": "java",
        "name": "Java",
        "version": "Java 17"
      },
      {
        "id": "python",
        "name": "Python",
        "version": "Python 3.10"
      }
    ]
  }
}
```

---

## 八、错误码定义

### 8.1 通用错误码

| 错误码 | 说明 |
|--------|------|
| 400001 | 参数错误 |
| 400002 | 参数缺失 |
| 400003 | 参数格式错误 |
| 401001 | Token过期 |
| 401002 | Token无效 |
| 401003 | 未登录 |
| 403001 | 权限不足 |
| 404001 | 用户不存在 |
| 404002 | 题目不存在 |
| 404003 | 提交记录不存在 |
| 404004 | 讨论不存在 |
| 409001 | 用户名已存在 |
| 409002 | 邮箱已注册 |
| 500001 | 服务器内部错误 |
| 500002 | 数据库错误 |
| 500003 | 评测系统错误 |

---

## 九、附录

### 9.1 支持的语言及版本

| 语言 | ID | 版本 | 编译命令 |
|------|-----|------|----------|
| C | c | C11 | gcc -std=c11 -O2 |
| C++ | cpp | C++17 | g++ -std=c++17 -O2 |
| Java | java | Java 17 | javac |
| Python | python | Python 3.10 | python3 |

### 9.2 评测状态说明

| 状态 | 说明 |
|------|------|
| pending | 等待评测 |
| judging | 评测中 |
| accepted | 答案正确（AC） |
| wrong_answer | 答案错误（WA） |
| time_limit_exceeded | 超时（TLE） |
| memory_limit_exceeded | 超内存（MLE） |
| runtime_error | 运行时错误（RE） |
| compile_error | 编译错误（CE） |

### 9.3 变更日志

| 版本 | 日期 | 变更内容 |
|------|------|----------|
| 1.0.0 | 2026-03-17 | 初始版本 |