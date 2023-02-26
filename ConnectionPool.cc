#include "ConnectionPool.h"

const std::string CONFIG_NAME = "mysql.cnf";

ConnectionPool *ConnectionPool::getInstance()
{
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::loadConfigFile()
{
    std::ifstream in(CONFIG_NAME);
    if (!in.is_open())
    {
        LOG(CONFIG_NAME + " file os not exist!");
        return false;
    }

    std::string line;
    while (std::getline(in, line))
    {
        // # 之后的为注释内容
        size_t pos = line.find('#');
        if (pos != std::string::npos)
            line.erase(line.begin() + pos, line.end());
        size_t idx = line.find('=');
        if (idx == std::string::npos)
        {
            continue;
        }
        else
        {
            size_t endidx = line.find('\n', idx);
            std::string key = line.substr(0, idx);
            std::string value = line.substr(idx + 1, endidx - idx - 1);
            // std::cout << key << " : " << value  << std::endl;
            if (key == "ip")
                ip_ = value;
            else if (key == "port")
                port_ = std::stoi(value);
            else if (key == "username")
                username_ = value;
            else if (key == "password")
                password_ = value;
            else if (key == "dbname")
                dbname_ = value;
            else if (key == "initSize")
                initSize_ = std::stoul(value);
            else if (key == "maxSize")
                maxSize_ = std::stoul(value);
            else if (key == "maxIdleTime")
                maxIdleTime_ = std::stoi(value);
            else if (key == "connectionTimeOut")
                connectionTimeout_ = std::stoi(value);
            else
                continue;
        }
    }
    return true;
}

ConnectionPool::ConnectionPool()
{
    // 加载配置项
    if (!loadConfigFile())
    {
        LOG(CONFIG_NAME + " config file read failed");
        return;
    }

    // 创建初始的连接
    for (int i = 0; i < initSize_; ++i)
    {
        Connection *p = new Connection();
        p->connect(ip_, port_, username_, password_, dbname_);
        p->refreshAliveTime(); // 刷新开始空闲的事件
        connectionQue_.push(p);
        connectionCnt_++;
    }

    // 启动一个线程，作为连接的生产者
    std::thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();

    // 启动一个定时线程，扫描超过maxIdleTime的空闲连接，对于连接回收
    std::thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

void ConnectionPool::produceConnectionTask()
{
    for (;;)
    {
        std::unique_lock<std::mutex> lock(queueMtx_);
        while (!connectionQue_.empty())
        {
            cv.wait(lock); // 队列不空，生成线程进入等待状态
        }

        // 数量没有达到上限，继续创建新线程
        if (connectionCnt_ < maxSize_)
        {
            Connection *p = new Connection();
            p->connect(ip_, port_, username_, password_, dbname_);
            p->refreshAliveTime(); // 刷新开始空闲的事件
            connectionQue_.push(p);
            connectionCnt_++;
        }

        // 通知消费者消费连接
        cv.notify_all();
    }
}

std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> lock(queueMtx_);
    while (connectionQue_.empty())
    {
        std::cv_status status = cv.wait_for(lock, std::chrono::milliseconds(connectionTimeout_));
        if(status == std::cv_status::timeout && connectionQue_.empty())
        {
            LOG("获取空闲连接超时 获取失败");
            return nullptr;
        }
    }
    std::shared_ptr<Connection> sp(connectionQue_.front(), [&](Connection* pcon) {
        std::unique_lock<std::mutex> lock(queueMtx_);
        pcon->refreshAliveTime();   // 刷新空闲时间
        connectionQue_.push(pcon); // 把使用完的线程放到到线程队列中
    });
    connectionQue_.pop();
    cv.notify_all();
    return sp;
}

void ConnectionPool::scannerConnectionTask()
{
    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(maxIdleTime_));

        // 扫描整个队列，释放多余的连接
        std::unique_lock<std::mutex> lock(queueMtx_);
        while(connectionCnt_ > initSize_)
        {
            Connection* p = connectionQue_.front();
            if(p->getAliveTime().count() > maxIdleTime_)
            {
                connectionQue_.pop();
                connectionCnt_--;
                delete p;
            }
            else 
            {
                break; // 对头的连接都没有超过，其他肯定也没有
            }
        }
    }
}