# 公共服务模块使用文档（common）

## 1. 模块定位

`common` 为 OJ 各业务模块提供统一基础能力，当前包含：

- 配置管理（环境变量读取与默认值）
- 日志能力（统一日志级别与输出）
- MySQL 连接池封装（可选启用）
- Redis 缓存封装（可选启用）
- 错误 JSON 输出工具
- 基础设施启动/关闭编排（bootstrap）

当前建议：`judge`、`discussion`、`rank` 直接链接 `oj_common`；`management`（Java）保持配置语义对齐，不直接调用 C++ 接口。

---

## 2. 目录结构

```text
common/
├── include/oj/
│   ├── bootstrap.h
│   ├── config.h
│   ├── json_error.h
│   ├── log.h
│   ├── mysql_pool.h
│   └── redis_cache.h
├── src/
│   ├── bootstrap.cpp
│   ├── config.cpp
│   ├── json_error.cpp
│   ├── log.cpp
│   ├── mysql_pool.cpp
│   └── redis_cache.cpp
├── tests/
│   └── common_smoke_test.cpp
└── CMakeLists.txt
```

---

## 3. 与评测模块（judge）对齐的构建方式

与 `评测模块使用文档` 一致，统一使用根目录 CMake 构建。

在仓库根目录执行：

```powershell
New-Item -ItemType Directory -Force -Path build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

如果只验证 common 冒烟测试：

```powershell
cmake --build . --target common_smoke_test
.\common\common_smoke_test.exe
```

> 若路径含中文导致 MinGW/Ninja 路径问题，可临时映射盘符后构建：

```powershell
subst X: "d:\你的项目路径\OJ-direct-test"
cmake -S X:/ -B X:/build-ascii -G Ninja
cmake --build X:/build-ascii --target common_smoke_test
X:/build-ascii/common/common_smoke_test.exe
subst X: /D
```

---

## 4. 对外接口（供业务模块接入）

### 4.1 配置加载

```cpp
#include "oj/config.h"

oj::AppConfig cfg = oj::loadConfigFromEnv();
```

### 4.2 基础设施生命周期

```cpp
#include "oj/bootstrap.h"

oj::initInfrastructure(cfg);
// ... 业务逻辑
oj::shutdownInfrastructure();
```

### 4.3 日志

```cpp
#include "oj/log.h"

OJ_LOG_INFO("service started");
OJ_LOG_WARN("cache unavailable");
OJ_LOG_ERROR("db connect failed");
```

### 4.4 错误 JSON

```cpp
#include "oj/json_error.h"

std::string body = oj::makeErrorJson("BAD_REQUEST", "invalid parameter");
```

### 4.5 MySQL 连接池

```cpp
#include "oj/mysql_pool.h"

auto& pool = oj::MysqlConnectionPool::instance();
if (pool.available()) {
    void* conn = pool.acquire();
    // 使用后务必释放
    pool.release(conn);
}
```

### 4.6 Redis 缓存

```cpp
#include "oj/redis_cache.h"

auto& cache = oj::RedisCache::instance();
if (cache.connected()) {
    cache.set("k", "v", 60);
}
```

---

## 5. 统一环境变量规范（项目级）

### 5.1 MySQL

| 变量 | 默认值 | 说明 |
|---|---|---|
| `OJ_MYSQL_HOST` | `127.0.0.1` | MySQL 主机 |
| `OJ_MYSQL_PORT` | `3306` | MySQL 端口 |
| `OJ_MYSQL_USER` | `oj` | 用户名 |
| `OJ_MYSQL_PASSWORD` | 空 | 密码 |
| `OJ_MYSQL_DATABASE` | `myOJ` | 数据库名 |
| `OJ_MYSQL_POOL_MIN` | `2` | 连接池最小连接数 |
| `OJ_MYSQL_POOL_MAX` | `16` | 连接池最大连接数 |

### 5.2 Redis

| 变量 | 默认值 | 说明 |
|---|---|---|
| `OJ_REDIS_HOST` | `127.0.0.1` | Redis 主机 |
| `OJ_REDIS_PORT` | `6379` | Redis 端口 |
| `OJ_REDIS_PASSWORD` | 空 | Redis 密码（可选） |
| `OJ_REDIS_DB` | `0` | 逻辑库索引 |

### 5.3 日志

| 变量 | 默认值 | 说明 |
|---|---|---|
| `OJ_LOG_FILE` | 空 | 日志文件路径；为空时仅控制台 |
| `OJ_LOG_LEVEL` | `info` | `debug/info/warning/error` |

---

## 6. 可选依赖开关与行为

### 6.1 启用 MySQL Connector/C++

```powershell
cmake .. -G "MinGW Makefiles" -DOJ_ENABLE_MYSQL=ON -DMYSQL_CONCPP_INCLUDE="C:/path/to/mysql-connector/include" -DMYSQL_CONCPP_LIB="C:/path/to/mysqlcppconn.lib"
```

### 6.2 启用 Redis（hiredis）

```powershell
cmake .. -G "MinGW Makefiles" -DOJ_ENABLE_REDIS=ON
```

### 6.3 未启用依赖时

- MySQL/Redis 会进入降级状态（`available()/connected()` 为 `false`）
- 启动时输出 WARN
- 这是预期行为，不视为故障

---

## 7. 与其他模块的对接约定

### judge / discussion / rank（C++）

1. 在 CMake 中链接 `oj_common`
2. 程序入口统一执行：
   - `cfg = loadConfigFromEnv()`
   - `initInfrastructure(cfg)`
3. 程序退出统一执行：`shutdownInfrastructure()`
4. 新增代码不再重复定义 MySQL/Redis/日志环境变量名

### management（Java）

- 不直接调用 common C++ 接口
- 保持 `spring.datasource` 读取同一套 `OJ_MYSQL_*` 环境变量
- 连接池参数语义对齐：
  - `OJ_MYSQL_POOL_MIN` ↔ `hikari.minimum-idle`
  - `OJ_MYSQL_POOL_MAX` ↔ `hikari.maximum-pool-size`

---

## 8. 冒烟测试说明

`common_smoke_test` 当前覆盖：

- 配置环境变量读取
- 错误 JSON 转义
- 日志文件写入
- bootstrap 启停流程
- MySQL/Redis 在未启用依赖时的降级行为

运行方式：

```powershell
cmake --build . --target common_smoke_test
.\common\common_smoke_test.exe
```

通过标准：输出 `Common smoke tests passed.`

---

## 9. 当前状态与边界

- common 核心功能已实现并可接入。
- 当前定位是“功能可用版”，非性能压测版。
- 若后续调整环境变量名/默认值，需同步更新本文件与各模块配置。
