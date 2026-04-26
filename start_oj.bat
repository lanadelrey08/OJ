@echo off

rem OJ 系统启动脚本

setlocal enabledelayedexpansion

rem 检查环境
call :check_environment
if errorlevel 1 goto :eof

rem 编译C++模块
call :build_cpp
if errorlevel 1 goto :eof

rem 编译Java模块
call :build_java
if errorlevel 1 goto :eof

rem 启动各个模块
call :start_modules

pause
goto :eof

:check_environment
echo 检查环境...

rem 检查JDK
java -version >nul 2>&1
if errorlevel 1 (
    echo 错误：未找到JDK，请确保JDK已安装并添加到环境变量
    exit /b 1
)

echo JDK 检查通过

rem 检查Maven
mvn -version >nul 2>&1
if errorlevel 1 (
    echo 错误：未找到Maven，请确保Maven已安装并添加到环境变量
    exit /b 1
)

echo Maven 检查通过

rem 检查MySQL
mysql --version >nul 2>&1
if errorlevel 1 (
    echo 警告：未找到MySQL命令行工具，但系统仍可能正常运行
) else (
    echo MySQL 检查通过
)

rem 检查Redis
redis-cli --version >nul 2>&1
if errorlevel 1 (
    echo 警告：未找到Redis命令行工具，但系统仍可能正常运行
) else (
    echo Redis 检查通过
)

echo 环境检查完成

:build_cpp
echo 编译C++模块...

if not exist build mkdir build
cd build

cmake .. -DOJ_ENABLE_MYSQL=ON -DOJ_ENABLE_REDIS=ON
if errorlevel 1 (
    echo 错误：CMake配置失败
    cd ..
    exit /b 1
)

cmake --build .
if errorlevel 1 (
    echo 错误：编译失败
    cd ..
    exit /b 1
)

cd ..
echo C++模块编译完成

:build_java
echo 编译Java模块...

cd management
mvn clean package
if errorlevel 1 (
    echo 错误：Java模块编译失败
    cd ..
    exit /b 1
)

cd ..
echo Java模块编译完成

:start_modules
echo 启动各个模块...

rem 启动管理模块
start "Management Module" cmd /c "cd management && java -jar target/problem-management-1.0.0.jar"

rem 等待1秒
ping localhost -n 2 >nul

rem 启动评测模块
start "Judge Module" cmd /c "cd build/judge && judge.exe"

rem 等待1秒
ping localhost -n 2 >nul

rem 启动讨论区模块
start "Discussion Module" cmd /c "cd build/discussion && discussion.exe"

rem 等待1秒
ping localhost -n 2 >nul

rem 启动排行榜模块
start "Rank Module" cmd /c "cd build/rank && rank.exe"

rem 等待1秒
ping localhost -n 2 >nul

rem 启动登录模块
start "Login Module" cmd /c "cd datas/login && mvn spring-boot:run"

echo 所有模块已启动

echo 系统启动完成！
echo 管理模块：http://localhost:8083
echo 评测模块：http://localhost:8080
echo 讨论区模块：http://localhost:8081
echo 排行榜模块：http://localhost:8082
echo 登录模块：http://localhost:8084

goto :eof