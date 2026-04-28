# 题目管理模块

## 功能概览

- 题目分页查询
- 题目详情查询
- 管理员创建题目
- 管理员更新题目
- 管理员删除题目
- 标签维护
- 测试用例维护
- JWT 鉴权
- 统一响应体
- 统一异常处理
- 单元测试与覆盖率配置

## 技术栈

- Java 8 语法级别
- Spring Boot 2.7.18
- Spring Web
- Spring Validation
- Spring Data JPA
- MySQL
- H2（测试环境）
- JUnit 5
- Mockito
- MockMvc
- JaCoCo

## 项目结构

```text
E:\OJ
├── src
│   ├── README
│   ├── main
│   │   ├── java\com\oj\problem
│   │   │   ├── common
│   │   │   ├── controller
│   │   │   ├── dto
│   │   │   ├── entity
│   │   │   ├── exception
│   │   │   ├── repository
│   │   │   ├── security
│   │   │   ├── service
│   │   │   └── ProblemManagementApplication.java
│   │   └── resources
│   │       ├── application.yml
│   │       └── schema.sql
│   └── test
│       ├── java\com\oj\problem
│       └── resources\application-test.yml
```

## 主要接口

### 公开接口

- `GET /api/v1/problems`
- `GET /api/v1/problems/{id}`

### 管理员接口

- `POST /api/v1/problems`
- `PUT /api/v1/problems/{id}`
- `DELETE /api/v1/problems/{id}`

## 环境要求

- JDK 8 及以上
- Maven 3.8 及以上
- MySQL 8.0（运行主程序时）
- IntelliJ IDEA（推荐）

## 配置说明

### 运行配置

主程序配置文件：
[application.yml](E:/OJ/src/main/resources/application.yml)

为便于与 `common` 模块联调，management 运行时默认读取与 `common` 一致的数据库环境变量：

- `OJ_MYSQL_HOST`，默认 `localhost`
- `OJ_MYSQL_PORT`，默认 `3306`
- `OJ_MYSQL_DATABASE`，默认 `oj`
- `OJ_MYSQL_USER`，默认 `root`
- `OJ_MYSQL_PASSWORD`，默认 `password`

如果没有显式设置这些环境变量，management 会按上述默认值拼接 JDBC URL。

主要配置项包括：

- 服务端口：`8080`
- 接口上下文：`/api`
- MySQL 数据源
- JPA 自动建表/更新
- JWT 密钥

示例：

```yaml
server:
  port: 8080
  servlet:
    context-path: /api

spring:
  datasource:
    url: jdbc:mysql://${OJ_MYSQL_HOST:localhost}:${OJ_MYSQL_PORT:3306}/${OJ_MYSQL_DATABASE:oj}?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8
    username: ${OJ_MYSQL_USER:root}
    password: ${OJ_MYSQL_PASSWORD:password}
    driver-class-name: com.mysql.cj.jdbc.Driver

security:
  jwt:
    secret: oj-secret-key
```

### 测试配置

测试配置文件：
[application-test.yml](E:/OJ/src/test/resources/application-test.yml)

测试默认使用 H2 内存数据库，不依赖真实 MySQL。

## 数据库初始化

数据库脚本文件：
[schema.sql](E:/OJ/src/main/resources/schema.sql)

当前包含的核心表：

- `users`
- `problems`
- `test_cases`
- `tags`
- `problem_tags`

## 如何导入 IDEA

1. 打开 IDEA
2. 选择 `Open`
3. 打开项目目录 `E:\OJ`
4. 如果没有自动识别为 Maven 项目，右键 [pom.xml](E:/OJ/pom.xml)
5. 选择 `Add as Maven Project`
6. 配置 `Project SDK` 为 JDK 8 或更高版本

## 如何运行项目

### 方式一：Maven

在项目根目录执行：

```bash
set OJ_MYSQL_HOST=127.0.0.1
set OJ_MYSQL_PORT=3306
set OJ_MYSQL_DATABASE=oj
set OJ_MYSQL_USER=root
set OJ_MYSQL_PASSWORD=password
mvn spring-boot:run
```

如果你已经为 `common` 模块配置好了上述变量，management 可以直接复用同一套数据库连接信息。

### 方式二：IDEA

直接运行：
[ProblemManagementApplication.java](E:/OJ/src/main/java/com/oj/problem/ProblemManagementApplication.java)

启动后访问：

```text
http://localhost:8080/api/v1/problems
```

## 如何运行测试

### 在 IDEA 中运行

右键 `src/test/java` 目录，选择：

- `Run 'All Tests'`

### 使用 Maven 运行

```bash
mvn clean test
```

## 如何查看覆盖率

推荐使用 Maven + JaCoCo：

```bash
mvn clean verify
```

覆盖率报告生成位置：

[JaCoCo 报告](E:/OJ/target/site/jacoco/index.html)

说明：

- 项目已配置 JaCoCo
- 行覆盖率门槛为 `80%`
- 若未达到 80%，`verify` 会失败

## 已编写测试

主要测试文件：

- [ProblemServiceImplTest.java](E:/OJ/src/test/java/com/oj/problem/service/impl/ProblemServiceImplTest.java)
- [ProblemControllerTest.java](E:/OJ/src/test/java/com/oj/problem/controller/ProblemControllerTest.java)
- [JwtTokenServiceTest.java](E:/OJ/src/test/java/com/oj/problem/security/JwtTokenServiceTest.java)
- [GlobalExceptionHandlerTest.java](E:/OJ/src/test/java/com/oj/problem/exception/GlobalExceptionHandlerTest.java)
- [ApiResponseTest.java](E:/OJ/src/test/java/com/oj/problem/common/ApiResponseTest.java)

## 认证说明

管理员接口需要在请求头中携带 JWT：

```text
Authorization: Bearer <JWT_TOKEN>
```

JWT payload 至少包含：

```json
{
  "user_id": 1,
  "role": "admin"
}
```

## 注意事项

1. 公开题目详情接口不会返回完整测试用例
2. 运行主程序时需要真实 MySQL
3. 跑测试时不需要真实 MySQL
4. 如果 IDEA 自带 Coverage 有问题，优先使用 Maven 的 JaCoCo 报告
5. 如果 `mvn clean verify` 报 JRE/JDK 错误，需要检查 `JAVA_HOME` 和 Maven 使用的 JDK

## 相关文档

- [题目管理模块实现说明.md](E:/OJ/题目管理模块实现说明.md)
- [测试说明.md](E:/OJ/测试说明.md)
- [API接口规范.md](E:/OJ/API接口规范.md)
- [架构设计文档.md](E:/OJ/架构设计文档.md)
- [数据模型设计.md](E:/OJ/数据模型设计.md)

## 当前实现边界

本次新增部分只实现题目管理模块，适合作为课程实验或 OJ 后端子模块示例。  
如果后续需要继续扩展，可以在此基础上补充：

- 用户模块
- 提交评测模块
- 排行榜模块
- 讨论区模块
- Swagger/OpenAPI 文档
- 集成测试
