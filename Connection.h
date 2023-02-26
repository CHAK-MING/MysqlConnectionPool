#pragma once

/**
 * 实现Mysql数据库的操作
 * 
*/

#include "public.h"
#include <mysql/mysql.h>
#include <string>
#include <chrono>

class Connection
{
public:
    // 初始化数据库连接
    Connection();
    // 释放数据库连接
    ~Connection();
    // 连接数据库
    bool connect(std::string ip, uint16_t port,
        std::string user, std::string password,
        std::string dbname);
    // 更新操作 insert delete update
    bool update(std::string sql);
    // 查询操作
    MYSQL_RES* query(std::string sql);

    // 刷新起始的空闲时间点 
    inline void refreshAliveTime() { alivetime_ = std::chrono::high_resolution_clock::now(); }
    // 返回存活的时间
    inline std::chrono::seconds getAliveTime() const 
    { 
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - alivetime_); 
    }
private:
    MYSQL* conn_;
    std::chrono::system_clock::time_point alivetime_; // 记录进入空闲状态后的存活时间
};