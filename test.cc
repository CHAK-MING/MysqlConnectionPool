#include "ConnectionPool.h"

using namespace std;

int main()
{
    auto cp = ConnectionPool::getInstance();
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 5000; ++i)
    {
        char sql[1024] = {0};
        sprintf(sql, "insert into user(name,age,sex) values('%s','%d','%s')", "zhangsan", 18, "male");
       cp->getConnection()->update(sql);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

    // ConnectionPool::getInstance()->getConnection();

    // Connection conn;
    // cout << "请输入要执行的操作(update/select):> ";
    // string dowork;
    // string sql;
    // getline(cin, dowork);
    // cout << "请输入sql语句: >";
    // getline(cin, sql);
    // conn.connect("127.0.0.1", 3306, "root", "604222352mj", "chat");
    // if(dowork == "update")
    // {
    //     conn.update(sql);
    // }
    // else if(dowork == "select")
    // {
    //     auto res = conn.query(sql);
    // }

    return 0;
}