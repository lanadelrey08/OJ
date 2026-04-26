@echo off

rem OJ 系统停止脚本

echo 停止OJ系统模块...

rem 停止Java进程
for /f "tokens=2" %%i in ('tasklist /fi "imagename eq java.exe" /fo list /v ^| findstr "problem-management-1.0.0.jar"') do (
    echo 停止管理模块 (PID: %%i)
    taskkill /pid %%i /f
)

for /f "tokens=2" %%i in ('tasklist /fi "imagename eq java.exe" /fo list /v ^| findstr "LoginApplication"') do (
    echo 停止登录模块 (PID: %%i)
    taskkill /pid %%i /f
)

rem 停止C++进程
for /f "tokens=2" %%i in ('tasklist /fi "imagename eq judge.exe" /fo list') do (
    echo 停止评测模块 (PID: %%i)
    taskkill /pid %%i /f
)

for /f "tokens=2" %%i in ('tasklist /fi "imagename eq discussion.exe" /fo list') do (
    echo 停止讨论区模块 (PID: %%i)
    taskkill /pid %%i /f
)

for /f "tokens=2" %%i in ('tasklist /fi "imagename eq rank.exe" /fo list') do (
    echo 停止排行榜模块 (PID: %%i)
    taskkill /pid %%i /f
)

echo OJ系统已停止

pause