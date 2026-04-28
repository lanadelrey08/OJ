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

- Java 8
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
├── src
│   ├── main
│   │   ├── java/com/oj/problem
│   │   └── resources
│   │       ├── application.yml
│   │       ├── schema.sql
│   │       └── init-data.sql
│   └── test
│       ├── java/com/oj/problem
│       └── resources/application-test.yml
└── target
```

## 主要接口

### 公开接口

- `GET /api/v1/problems`
- `GET /api/v1/problems/{id}`
- `GET /api/v1/problems/{id}/test-cases`

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

运行配置文件：
[application.yml](E:/OJ/OJ/management/src/main/resources/application.yml)

为便于与 `common` 模块联调，management 运行时默认读取与 `common` 一致的数据库环境变量：

- `OJ_MYSQL_HOST`，默认 `127.0.0.1`
- `OJ_MYSQL_PORT`，默认 `3306`
- `OJ_MYSQL_DATABASE`，默认 `myOJ`
- `OJ_MYSQL_USER`，默认 `oj`
- `OJ_MYSQL_PASSWORD`，默认空字符串
- `OJ_MYSQL_POOL_MIN`，默认 `2`
- `OJ_MYSQL_POOL_MAX`，默认 `16`

如果没有显式设置这些环境变量，management 会按上述默认值拼接 JDBC URL，并用 HikariCP 读取同一套连接池参数。

主要配置项包括：

- `OJ_MANAGEMENT_PORT`
- `OJ_MANAGEMENT_CONTEXT_PATH`
- `OJ_JWT_SECRET`
- `OJ_JPA_DDL_AUTO`
- `OJ_SHOW_SQL`
- `OJ_MYSQL_HOST`
- `OJ_MYSQL_PORT`
- `OJ_MYSQL_DATABASE`
- `OJ_MYSQL_USER`
- `OJ_MYSQL_PASSWORD`
- `OJ_MYSQL_POOL_MIN`
- `OJ_MYSQL_POOL_MAX`

示例：

```yaml
server:
  port: ${OJ_MANAGEMENT_PORT:8080}
  servlet:
    context-path: ${OJ_MANAGEMENT_CONTEXT_PATH:/api}

spring:
  datasource:
    url: jdbc:mysql://${OJ_MYSQL_HOST:127.0.0.1}:${OJ_MYSQL_PORT:3306}/${OJ_MYSQL_DATABASE:myOJ}?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8
    username: ${OJ_MYSQL_USER:oj}
    password: ${OJ_MYSQL_PASSWORD:}
    driver-class-name: com.mysql.cj.jdbc.Driver
    hikari:
      minimum-idle: ${OJ_MYSQL_POOL_MIN:2}
      maximum-pool-size: ${OJ_MYSQL_POOL_MAX:16}

security:
  jwt:
    secret: ${OJ_JWT_SECRET:oj-management-dev-secret}
```

### 测试配置

测试配置文件：
[application-test.yml](E:/OJ/OJ/management/src/test/resources/application-test.yml)

测试默认使用 H2 内存数据库，不依赖真实 MySQL。

## 数据库初始化

建表脚本：
[schema.sql](E:/OJ/OJ/management/src/main/resources/schema.sql)

测试数据脚本：
[init-data.sql](E:/OJ/OJ/management/src/main/resources/init-data.sql)

当前核心表包括：

- `users`
- `problems`
- `test_cases`
- `tags`
- `problem_tags`

## 数据库使用方法

### 1. 创建数据库

先登录 MySQL：

```bash
mysql -u root -p
```

执行：

```sql
CREATE DATABASE IF NOT EXISTS myOJ CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE myOJ;
```

### 2. 导入表结构

在 `management` 目录下执行：

```bash
mysql -u root -p myOJ < src/main/resources/schema.sql
```

### 3. 导入测试数据

继续执行：

```bash
mysql -u root -p myOJ < src/main/resources/init-data.sql
```

这会导入一条公开题目 `Hello Problem`，可直接用于浏览器接口验证。

### 4. 配置数据库连接

如果你的 MySQL 密码不是空，启动前先设置环境变量，例如在 PowerShell 中：

```powershell
$env:OJ_MYSQL_HOST='127.0.0.1'
$env:OJ_MYSQL_PORT='3306'
$env:OJ_MYSQL_DATABASE='myOJ'
$env:OJ_MYSQL_USER='oj'
$env:OJ_MYSQL_PASSWORD='你的MySQL密码'
$env:OJ_MYSQL_POOL_MIN='2'
$env:OJ_MYSQL_POOL_MAX='16'
```

如需修改 JWT 密钥，也可以设置：

```powershell
$env:OJ_JWT_SECRET='your-own-secret'
```

## 如何导入 IDEA

1. 打开 IDEA
2. 选择 `Open`
3. 打开项目目录 `E:\OJ\OJ\management`
4. 如果没有自动识别为 Maven 项目，右键 [pom.xml](E:/OJ/OJ/management/pom.xml)
5. 选择 `Add as Maven Project`
6. 配置 `Project SDK` 为 JDK 8 或更高版本

## 如何运行项目

### 方式一：Maven

在项目根目录执行：

```bash
set OJ_MYSQL_HOST=127.0.0.1
set OJ_MYSQL_PORT=3306
set OJ_MYSQL_DATABASE=myOJ
set OJ_MYSQL_USER=oj
set OJ_MYSQL_PASSWORD=
set OJ_MYSQL_POOL_MIN=2
set OJ_MYSQL_POOL_MAX=16
mvn spring-boot:run
```

如果你已经为 `common` 模块配置好了上述变量，management 可以直接复用同一套数据库连接信息。

### 方式二：IDEA

直接运行：
[ProblemManagementApplication.java](E:/OJ/OJ/management/src/main/java/com/oj/problem/ProblemManagementApplication.java)

启动后访问：

```text
http://localhost:8080/api/v1/problems
```

如果已经导入了 `init-data.sql`，还可以直接访问：

```text
http://localhost:8080/api/v1/problems/1
```

获取某个题目的全部测试用例：
```text
http://localhost:8080/api/v1/problems/1/test-cases
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
[JaCoCo 报告](E:/OJ/OJ/management/target/site/jacoco/index.html)

说明：

- 项目已配置 JaCoCo
- 行覆盖率门槛为 `80%`
- 若未达到 `80%`，`verify` 会失败

## 已编写测试

主要测试文件：

- [ProblemServiceImplTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/service/impl/ProblemServiceImplTest.java)
- [ProblemControllerTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/controller/ProblemControllerTest.java)
- [JwtTokenServiceTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/security/JwtTokenServiceTest.java)
- [GlobalExceptionHandlerTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/exception/GlobalExceptionHandlerTest.java)
- [ApiResponseTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/common/ApiResponseTest.java)

测试用例接口已覆盖：
- `ProblemControllerTest#getProblemTestCasesShouldReturnAllTestCases`
- `ProblemServiceImplTest#getProblemTestCasesShouldReturnPublicProblemTestCases`
- `ProblemServiceImplTest#getProblemTestCasesShouldRejectPrivateProblem`

当前已通过验证：
```bash
mvn test
```

验证结果：
```text
Tests run: 30, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS
```

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

1. 可通过 `GET /api/v1/problems/{id}/test-cases` 获取某个公开题目的全部测试用例
2. 运行主程序时需要真实 MySQL
3. 跑测试时不需要真实 MySQL
4. 如果 IDEA 自带 Coverage 有问题，优先使用 Maven 的 JaCoCo 报告
5. 如果 `mvn clean verify` 报 JRE/JDK 错误，需要检查 `JAVA_HOME` 和 Maven 使用的 JDK

## 当前实现边界

本模块当前只实现题目管理功能，适合作为课程实验或 OJ 后端子模块示例。

后续如果需要继续扩展，可以在此基础上补充：

- 用户模块
- 提交评测模块
- 排行榜模块
- 讨论区模块
- Swagger / OpenAPI 文档
- 集成测试
