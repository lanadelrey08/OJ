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
management
├── pom.xml
├── README.md
├── 测试说明.md
├── src
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
│   │       ├── schema.sql
│   │       └── init-data.sql
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
[application.yml](E:/OJ/__publish_repo/management/src/main/resources/application.yml)

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
    url: jdbc:mysql://localhost:3306/oj?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8
    username: root
    password: password
    driver-class-name: com.mysql.cj.jdbc.Driver

security:
  jwt:
    secret: oj-secret-key
```

### 测试配置

测试配置文件：
[application-test.yml](E:/OJ/__publish_repo/management/src/test/resources/application-test.yml)

测试默认使用 H2 内存数据库，不依赖真实 MySQL。

## 数据库初始化

数据库脚本文件：
[schema.sql](E:/OJ/__publish_repo/management/src/main/resources/schema.sql)

测试数据脚本文件：
[init-data.sql](E:/OJ/__publish_repo/management/src/main/resources/init-data.sql)

当前包含的核心表：

- `users`
- `problems`
- `test_cases`
- `tags`
- `problem_tags`

## 数据库使用教程

如果你想把该模块实际跑起来，并通过浏览器验证接口是否能正常访问数据库，推荐按下面步骤操作。

### 1. 安装并启动 MySQL

- 推荐 MySQL 8.0
- 安装完成后确认以下命令可用：

```bash
mysql --version
```

### 2. 创建数据库

先登录 MySQL：

```bash
mysql -u root -p
```

输入密码后执行：

```sql
CREATE DATABASE IF NOT EXISTS oj CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE oj;
```

### 3. 导入表结构

在项目根目录 `management` 下执行：

```bash
mysql -u root -p oj < src/main/resources/schema.sql
```

这一步会创建：

- `users`
- `problems`
- `test_cases`
- `tags`
- `problem_tags`

### 4. 导入测试数据

继续执行：

```bash
mysql -u root -p oj < src/main/resources/init-data.sql
```

这个脚本会插入：

- 一个管理员用户
- 一条公开题目 `Hello Problem`
- 两条测试用例
- 一个标签 `入门`

### 5. 修改数据库连接配置

打开：
[application.yml](E:/OJ/__publish_repo/management/src/main/resources/application.yml)

确认下面配置与你本机 MySQL 一致：

```yaml
spring:
  datasource:
    url: jdbc:mysql://localhost:3306/oj?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8
    username: root
    password: 你的MySQL密码
```

### 6. 启动项目

在 `management` 目录下运行：

```bash
mvn spring-boot:run
```

或者直接在 IDEA 中运行：
[ProblemManagementApplication.java](E:/OJ/__publish_repo/management/src/main/java/com/oj/problem/ProblemManagementApplication.java)

### 7. 在浏览器中验证接口

项目启动成功后，可直接访问：

- 题目列表：
  [http://localhost:8080/api/v1/problems](http://localhost:8080/api/v1/problems)
- 题目详情：
  [http://localhost:8080/api/v1/problems/1](http://localhost:8080/api/v1/problems/1)

如果数据库与项目连接正常，你应该看到包含 `Hello Problem` 的 JSON 返回结果。

### 8. 快速排查

如果浏览器访问失败，优先检查：

1. MySQL 服务是否启动
2. `oj` 数据库是否创建成功
3. `schema.sql` 和 `init-data.sql` 是否已执行
4. `application.yml` 中用户名和密码是否正确
5. Spring Boot 项目是否启动成功

## 如何导入 IDEA

1. 打开 IDEA
2. 选择 `Open`
3. 打开项目目录 `management`
4. 如果没有自动识别为 Maven 项目，右键 [pom.xml](E:/OJ/__publish_repo/management/pom.xml)
5. 选择 `Add as Maven Project`
6. 配置 `Project SDK` 为 JDK 8 或更高版本

## 如何运行项目

### 方式一：Maven

在项目根目录执行：

```bash
mvn spring-boot:run
```

### 方式二：IDEA

直接运行：
[ProblemManagementApplication.java](E:/OJ/__publish_repo/management/src/main/java/com/oj/problem/ProblemManagementApplication.java)

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

[JaCoCo 报告](E:/OJ/__publish_repo/management/target/site/jacoco/index.html)

说明：

- 项目已配置 JaCoCo
- 行覆盖率门槛为 `80%`
- 若未达到 80%，`verify` 会失败

## 已编写测试

主要测试文件：

- [ProblemServiceImplTest.java](E:/OJ/__publish_repo/management/src/test/java/com/oj/problem/service/impl/ProblemServiceImplTest.java)
- [ProblemControllerTest.java](E:/OJ/__publish_repo/management/src/test/java/com/oj/problem/controller/ProblemControllerTest.java)
- [JwtTokenServiceTest.java](E:/OJ/__publish_repo/management/src/test/java/com/oj/problem/security/JwtTokenServiceTest.java)
- [GlobalExceptionHandlerTest.java](E:/OJ/__publish_repo/management/src/test/java/com/oj/problem/exception/GlobalExceptionHandlerTest.java)
- [ApiResponseTest.java](E:/OJ/__publish_repo/management/src/test/java/com/oj/problem/common/ApiResponseTest.java)

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

- [测试说明.md](E:/OJ/__publish_repo/management/测试说明.md)

## 当前实现边界

本次新增部分只实现题目管理模块，适合作为课程实验或 OJ 后端子模块示例。  
如果后续需要继续扩展，可以在此基础上补充：

- 用户模块
- 提交评测模块
- 排行榜模块
- 讨论区模块
- Swagger/OpenAPI 文档
- 集成测试
