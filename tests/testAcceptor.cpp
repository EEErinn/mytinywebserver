#include <iostream>

#include "../src/Acceptor.h"
#include "../src/InetAddress.h"
#include "../src/eventloop.h"

/**
 * @brief 验证事件绑定实现逻辑是否正确
 * 用两个终端 模拟客户端连接 nc 127.0.0.1 8080
 * 输出: 6 is connecting
 *       7 is connecting
 */

/**
 * Ctrl+Z 暂停程序及重启程序
 * Ctrl+Z - 暂停进程并放入后台
 * jobs - 显示当前暂停的进程
 * bg N 使第N个任务在后台运行
 * fg N 使第N个任务在前台运行
 * bg, fg 不带 N 时表示对最后一个进程操作
 * 当不小心按住ctrl + z 使得进程暂停 状态为terminate 怎么使其恢复前台运行
 * 例如在终端运行../bin/testAcceptor 可以通过jobs 查看后台运行进程 如下 : [1]+
 * 已停止../ bin / testAcceptor
 * 通过 fg 1 来恢复 fg % num
 * num是jobs中的第num个任务
 **/

using namespace mytinywebserver;

void m_newConnectionCallBack(int fd) {
    std::cout << fd << " is connecting" << std::endl;
}

int main() {
    EventLoop loop;
    Acceptor acc(&loop, InetAddress("127.0.0.1", 8080));
    acc.setNewconnectionCallBack(m_newConnectionCallBack);
    acc.listen();

    loop.loop();
    return 0;
}