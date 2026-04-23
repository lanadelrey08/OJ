# OJ 开发环境配置（公共服务模块）

## 1. 构建（Windows / MinGW 示例）

在仓库根目录 `OJ-direct-test` 下：

```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

- 仅评测子工程（旧方式）：也可在 `judge` 目录单独配置 CMake，脚本会自动加入上一级 `common` 库。

## 2. 运行 `judge_engine`

可执行文件一般在 `build/judge/judge_engine.exe`（若从根目录配置）或 `build/judge_engine.exe`（取决于生成器与 CMake 版本）。启动前请在**含 `judge/config/languages.json` 的工作目录**下运行，例如：

```powershell
cd d:\...\OJ-direct-test\judge
..\build\judge\judge_engine.exe
```

（路径按你本机 `build` 实际输出调整。）

## 3. 环境变量（公共服务）

| 变量 | 说明 | 默认 |
|------|------|------|
| `OJ_MYSQL_HOST` | MySQL 主机 | 127.0.0.1 |
| `OJ_MYSQL_PORT` | 端口 | 3306 |
| `OJ_MYSQL_USER` | 用户 | oj |
| `OJ_MYSQL_PASSWORD` | 密码 | 空 |
| `OJ_MYSQL_DATABASE` | 库名 | oj |
| `OJ_MYSQL_POOL_MIN` | 连接池最小连接数 | 2 |
| `OJ_MYSQL_POOL_MAX` | 连接池最大连接数 | 16 |
| `OJ_REDIS_HOST` | Redis 主机 | 127.0.0.1 |
| `OJ_REDIS_PORT` | Redis 端口 | 6379 |
| `OJ_REDIS_PASSWORD` | Redis 密码（可选） | 空 |
| `OJ_REDIS_DB` | 逻辑库索引 | 0 |
| `OJ_LOG_FILE` | 日志文件路径（可选） | 空（仅控制台） |
| `OJ_LOG_LEVEL` | debug / info / warning / error | info |

未开启 CMake 选项时：**MySQL 连接池与 Redis 客户端以占位方式编译**，进程可启动，`/health` 中 `mysql_pool_ready` / `redis_ready` 为 `false`。

## 4. 启用 MySQL Connector/C++

配置 CMake：

```powershell
cmake .. -G "MinGW Makefiles" -DOJ_ENABLE_MYSQL=ON -DMYSQL_CONCPP_INCLUDE="C:/path/to/mysql-connector/include" -DMYSQL_CONCPP_LIB="C:/path/to/mysqlcppconn.lib"
```

或使用 vcpkg 安装 `mysql-connector-cpp` 后，按 vcpkg 文档设置 toolchain，并打开 `OJ_ENABLE_MYSQL`。

若头文件路径与 `cppconn/*.h`、`mysql_driver.h` 不一致，请修改 `common/src/mysql_pool.cpp` 中的 `#include` 或追加 `target_include_directories`。

## 5. 启用 hiredis（Redis）

```powershell
cmake .. -DOJ_ENABLE_REDIS=ON
```

需已安装 hiredis 并在 CMake 中能找到目标 `hiredis::hiredis` 或库文件 `hiredis`。

## 6. 健康检查与前端联调

- `GET /health`：返回 JSON，包含 MySQL 池、Redis 是否就绪及池统计。
- 已启用 Crow **全局 CORS**（`Access-Control-Allow-Origin: *` 等），便于 Vue 开发服务器跨域调用 API。

## 7. Docker 一键起 MySQL + Redis（可选）

```yaml
# 示例 docker-compose.yml 可自行保存到项目外
services:
  mysql:
    image: mysql:8.0
    environment:
      MYSQL_ROOT_PASSWORD: root
      MYSQL_DATABASE: oj
      MYSQL_USER: oj
      MYSQL_PASSWORD: ojpass
    ports: ["3306:3306"]
  redis:
    image: redis:7
    ports: ["6379:6379"]
```

启动后设置 `OJ_MYSQL_PASSWORD=ojpass` 等与上面一致，再打开 `OJ_ENABLE_MYSQL` / `OJ_ENABLE_REDIS` 重新 CMake 编译。
