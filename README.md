# mytinywebserver

## 目录结构
- bin: 可执行文件
- build: cmake的产物
- src: 源代码
- lib: 链接文件
- tests: 测试文件
- CMakeLists.txt: cmake文件

## 项目介绍
项目背景：轻量级、高并发的Http服务器
项目目标：通用网络库、扩展容易、性能高效
关键技术：
    1. 使用epoll+LT的多路复用技术，使用主从reactor事件驱动模型
    2. 使用多核多线程提高并发量，并降低频繁创建线程的开销
    3. 基于双缓冲区技术实现异步日志
    4. 使用有限状态机解析http报文，目前暂只支持get请求
    5. 使用优先队列管理定时器，使用标记删除，处理不活跃的连接。
结果： 短连接支持2.7wQPS

## 开发流程
已完成
- 实现事件循环机制，完成事件处理 即eventloop、poller、channel
- 完成事件注册 即Acceptor、socket、InetAddress
- 完成tcp连接 即tcpserver、tcpconnection、buffer
- 多线程 即thread、eventthread、eventthreadpool
- 支持http1.1 即httpserver、httpcontext、httprequest、httpresponse
- 定时器
- 同步/异步日志
- webbench压测

正在开发
- 支持post请求, 解析普通文本格式
- 支持文件上传和下载

待完成
- 充分压测，与其他服务器对比
- 内存泄漏检查
- http cookie、session、servlet
- 配置系统config

未来计划
- 基于该框架，重构一个简易的前后端分离的实验室项目
- 使用redis存储session、mysql存储数据
- 支持https(待学)
- 支持代理(待学)
- 协程(待学)

### bug解决方案
1. 客户端断开了，服务器就断开了
因为客户端断开，服务器还在往客户端写，一直写，触发SIGPIPE信号，默认处理函数就是kill掉服务器。

2. 当在浏览器请求http://127.0.0.1:8080/hello等，服务器程序发送完reponse给服务器后，在处理下一个读事件，handleRead时段错误
首先，找到core dump文件。参考:[https://www.pudn.com/news/6292ee86e74b9677e8e0a8c8.html].注意，在生成core dump文件时，即使设置了ulimit -c unlimited，也先查一下ulimit -c 看是否为 unlimited.
    调试core dump， 命令为 gdb ../bin/testHttpServer /var/core/core-testHttpServer-11871-18446744073709551615
    bt查看堆栈信息，不断cotinue
    定位是TcpConnection::handleRead，Buffer的读指针没有移动2位。在请求头结束，还有一个\r\n没有处理。

3. 压测时，请求大部分都失败了，最后段错误
gdb调试core dump定位到定时器的部分。分析代码，发现添加TimerNode忘记加锁了。

4. 并发量不高
- listen的backlog参数设置为1024
- 进程可打开的文件描述符设置成65535，ulimit
- 开启o2编译器优化

...

## 项目中遇到的问题总结
> 在testEvent中，在每个线程中使用std::cout打印Cureent::tid(),但无法显示输出。
- 通过gdb调试，b 38 b 25 r n n， 可知代码逻辑正确。但直接执行testEvent则不显示, 定位是std::cout的原因。经分析，是epoll_wait阻塞，因此在每次std::cout时都需要强制从缓冲区取出数据。std::cout.flush()。

> muduo为什么poller需要拥有eventloop? 
- 为了在poller中检查当前线程是否是创建所属eventloop的线程。目前认为不重要。因为poller只被eventloop拥有，而eventloop的函数基本保证时线程安全的，所以本人在poller中没有增加eventloop成员变量。

> 如何保证线程安全？
- 参考muduo。每个工作线程有一个eventloop对象，不断循环监听事件。新连接到来时，主线程为其创建一个连接对象，将该连接对象会被分派给某个特定进程。此后，连接的所有操作都用该I/O线程管理，包括连接的读写、关闭等。
通过runInloop和queueInloop函数，可以让其他线程执行连接对象操作时是线程安全的。就是他认为一个对象只能在一个线程中被操作，那么就是线程安全。所以通过runInloop函数，让执行这个对象的I/O操作时，判断是不是在所属的线程，在则直接执行。不在，则将该操作注册到所属线程的队列中，并通知该线程，让他来执行这个操作。

> 多线程访问同一个对象时，对象被析构了，该如何处理？
智能指针

> std::bind
- A: https://blog.csdn.net/Jxianxu/article/details/107382049 第一个参数被占位符占用，表示这个参数以调用时传入的参数为准
- 影响绑定对象的生命周期 https://segmentfault.com/q/1010000018259726

> 为什么tcpconnection需要buffer?
- 发送缓冲区: 因为发送方可能发送不完数据，内核发送缓冲区就满了。因此把剩下的存在buffer里，注册写事件，可写的时候再写。
- 接收缓冲区: 因为收到的信息可能不完整，tcp是字节流服务，因此需要存起来等接收到完整数据再通知业务处理。

> 为什么tcpconnection需要使用enable_shared_from_this?
- 防止访问失效的对象或者发生网络串话
- 不使用shared_ptr类型指针，用func = std::bind()会保存原始的this，当B调用func，绑定的this对象已经析构，则会core dump。因此，保存shared_ptr让A的生命周期大于等于B。

> 什么时候服务端断开连接？主动断开连接和被动断开连接？
- 被动断开: 对端正常关闭，发fin, 服务器read返回0
- 服务端主动关闭连接，需要等到应用层缓冲区数据发送到客户端，才开始关闭

> 被动关闭什么时候调用close
- tcpserver将连接从map中移除，调用connectionDestoryed，将channel对应移除。然后tcpconnection开始析构，socket调用自身析构函数，才会调用close(fd)

> 客户端异常崩溃，服务器此时正在读，正在写，再等待三种情况的反应？
1. 正在读，read 错误为EINTR说明读是由中断引起的，如果是ECONNREST表示网络连接出了问题。
2. 正在写，会收到客户端发来的RST，write返回EPIPE，错误为EINTR表示在写的时候出现了中断错误 
3. EWOULDBLOCK写满了 EAGAIN读满了 这两个错误在非阻塞中可以忽略

> ET和LT触发POLLIN、POLLOUT的情况
- 参考资料：1.https://blog.csdn.net/daaikuaichuan/article/details/8877727
- 读事件POLLIN触发 内核接收缓冲区 三个情况 1.接收缓冲区数据从无到有 2.新数据被读入，缓冲区数据增多 3.接收缓冲区有数据，可读
  引起ET触发的情况: 1、2
  引起LT触发的情况: 1、2、3
  由于接收缓冲区中有数据，而ET无法触发。那么读取数据，不把缓冲区数据读完，只读一点，此时缓冲区有剩余数据，且没有新数据到达的话，是不会触发POLLIN来读取剩余的数据的。
- 写事件POLLOUT触发 内核发送缓冲区 三种情况 1.发送缓冲区从满到不满 2.旧数据被送走，缓冲区数据减少 3.发送缓冲区不满，有位置可写
  引起ET触发的情况: 1、2
  引起LT触发的情况: 1、2、3
  由于发送缓冲区不满，有位置可写，而ET无法触发。那么写入数据，不把缓冲区写满，只写一点，此时用户还有剩余要写入缓冲区的数据，但旧数据没有被送走时，即使有空间可写，不会触发POLLOUT来把剩余的数据写入。这样就无法保证用户数据的写入了。
所以，ET需要循环写、循环读。

> 为什么ET需要循环读？
- ET使用read读取，ssize_t read(int fd, void *buf, size_t count); read一次时，count设置比接收缓冲区的数据大小要小，那么读完整个接收缓冲区需要多次read。直到read的返回值比count小，或者read返回-1，errno设置为EAGAIN。不过，现在有这样的情况
reciveSize = read(sockfd, buffer, BUF_LEN);
reciveSize小于BUF_LEN的时候，是可以跳出read的循环，因为在读一次就会出现EAGAIN。所以，返回值小于请求值就可以返回了。