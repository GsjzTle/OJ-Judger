# OJ-Judger

> 周末在家无聊手撸了一个简易版判题服务器+判题核心（采用生产者消费者模式，未用上消息队列）的 demo。
>

~~由于偷懒~~未用 `cmake` 构建。

### 判题服务器：

+ 作为生产者，定期（$3$ seconds）从数据库中拉去任务（where result = 0）。
+ 多线程开发，以并发执行拉去任务、分配任务。

### 判题核心

作为消费者，与判题服务器建立 TCP 连接以接受任务并执行。

### 安装

`git clone https://gitee.com/gsjztle/oj-judger.git`

### 配置

打开 `JudgerServer.cpp`，修改数组库配置信息

### 编译 + 运行

+ `g++ JudgerServer.cpp -o JudgeServer -lpthread -std=c++11 -O2`：编译判题服务器
+ `g++ JudgerCore.cpp -o JudgeCore -lpthread -std=c++11 -O2`：编译判题核心
+ `./JudgerServer`：运行判题服务器
+ `./JudgerCore`：运行判题核心







