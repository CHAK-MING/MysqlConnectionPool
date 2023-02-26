#pragma once

/*
    连接池实现
*/

#include "public.h"
#include "Connection.h"
#include <mysql/mysql.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <functional>

class ConnectionPool
{
public:
    static ConnectionPool *getInstance();

    // 给外部提供接口，从连接池获取一个可用的空闲连接
    std::shared_ptr<Connection> getConnection();

private:
    ConnectionPool();
    // 读取配置文件
    bool loadConfigFile();
    // 运行在独立的线程中，专门产生新连接
    void produceConnectionTask();
    // 扫描超过maxIdleTime时间的空闲连接，对于连接的回收
    void scannerConnectionTask();

private:
    std::string ip_;                         // mysql的IP地址
    uint16_t port_;                          // mysql的端口号
    std::string username_;                   // mysql登录用户名
    std::string password_;                   // mysql登录密码
    std::string dbname_;                     // 连接的数据库名
    size_t initSize_;                        // mysql的初始连接数量
    size_t maxSize_;                         // mysql的最大连接数量
    int maxIdleTime_;                        // mysql的最大空闲时间
    int connectionTimeout_;                  // mysql的连接超时时间
    std::queue<Connection *> connectionQue_; // mysql存储连接的队列
    std::atomic_int connectionCnt_;          // 记录连接所创建的总数量
    std::mutex queueMtx_;                    // 维护连接队列的互斥锁

    std::condition_variable cv;              // 用于连接生成线程和消费现场的通信
};