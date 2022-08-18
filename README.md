# mytinywebserver

## 目录结构
- bin: 可执行文件
- build: cmake的产物
- src: 源代码
- lib: 链接文件
- tests: 测试文件
- CMakeLists.txt: cmake文件

## 开发步骤
*基本功能(尽量8月1号之前)
- 实现事件循环机制，完成事件处理 即eventloop、poller、channel
- 完成事件注册 即Acceptor、socket、InetAddress
- 完成tcp连接 即tcpserver、tcpconnection、buffer
- 多线程 即thread、eventthread、eventthreadpool
- 支持http1.1 url 即httpserver、httpcontext、httprequest、httpresponse
- 压测webbench、内存泄漏检查

*其他辅助功能(可以延期)
- 定时器
- 异步日志
- 优雅关闭

扩展功能
- *http cookie、session、serlvet
- *支持cgi、解析普通文本、json格式等数据
- *配置系统config
- *支持文件上传和下载
- 协程(学习)

未来功能
- *基于该框架，实现一个前后端分离的增删改查项目
- 用redis从session、mysql数据
- 支持https
- 支持代理
 
Note: * 为必须完成的功能

## 目前进度
1. 完成事件处理，eventloop - poller - channel 3日
2. 完成事件注册 即Acceptor、socket、InetAddress 1日
3. 完成tcp连接 即tcpserver、tcpconnection、buffer 6日
4. 多线程 即thread、eventthread、eventthreadpool
5. 支持http1.1 url 即httpserver、httpcontext、httprequest、httpresponse
6. 同步日志
7. 基于最小堆(优先队列)的定时器处理不活跃连接 2天

Todos:
8. 压测，内存检查




## 事件处理遇到的问题
> Q: muduo为什么poller需要拥有eventloop? 
- A: 为了在poller中检查当前线程是否是创建所属eventloop的线程。目前认为不重要。因为poller只被eventloop拥有，而eventloop的函数基本保证时线程安全的，所以本人在poller中没有增加eventloop成员变量

> Q: eventloop - poller - channel怎么保证线程安全？
- A：代填坑

> Q: bug: 在testEvent中，在每个线程中使用std::cout打印Cureent::tid(),但无法显示输出。
- A: 通过gdb调试，b 38 b 25 r n n， 可知代码逻辑正确。但直接执行testEvent则不显示, 定位是std::cout的原因。经分析，是epoll_wait阻塞，因此在每次std::cout时都强制从缓冲区取出数据。std::cout.flush()。

## tcp连接遇到的问题
> Q: 类的成员变量应该使用引用类型、指针类型还是对应类型对象呢？
- A: https://blog.csdn.net/bumingchun/article/details/112755993?spm=1001.2101.3001.6661.1&utm_medium=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1-112755993-blog-124997514.pc_relevant_multi_platform_featuressortv2removedup&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1-112755993-blog-124997514.pc_relevant_multi_platform_featuressortv2removedup&utm_relevant_index=1

> std::bind 绑定成员函数的用法
- A: https://blog.csdn.net/Jxianxu/article/details/107382049 第一个参数被占位符占用，表示这个参数以调用时传入的参数为准
- 影响绑定对象的生命周期 https://segmentfault.com/q/1010000018259726

> 为什么tcpconnection需要buffer?
- A: 因为发送方可能发送不完整数据，那么接收方用户层设置buffer能进行判断

> 为什么tcpconnection需要使用enable_shared_from_this?
- 防止访问失效的对象或者发生网络串话
- 不使用shared_ptr类型指针，用func = std::bind()会保存原始的this，当B调用func，绑定的this对象已经析构，则会core dump。因此，保存shared_ptr让A的生命周期大于等于B。

> make_shared 和 new 的区别？

> 什么时候服务端断开连接？主动断开连接和被动断开连接？
- 对端正常关闭，发fin, 服务器read返回0
- 服务端主动关闭连接，需要等到应用层缓冲区数据发送到客户端，才开始关闭

> 被动关闭什么时候调用close
- 当tcpconnection析构时，socket调用自身析构函数，才会调用close

客户端异常崩溃，服务器此时正在读，正在写，再等待三种情况的反应？
1. 正在读，read 错误为EINTR说明读是由中断引起的，如果是ECONNREST表示网络连接出了问题。
2. 正在写，会收到客户端发来的RST，write返回EPIPE，错误为EINTR表示在写的时候出现了中断错误 
3. EWOULDBLOCK写满了 EAGAIN读满了 这两个错误在非阻塞中可以忽略

----------------------


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

Q：为什么ET需要循环读？
A：ET使用read读取，ssize_t read(int fd, void *buf, size_t count); read一次时，count设置比接收缓冲区的数据大小要小，那么读完整个接收缓冲区需要多次read。
直到read的返回值比count小，或者read返回-1，errno设置为EAGAIN

https://www.169it.com/tech-qa-linux/article-7879993814782557919.html
如果用ET模式，要一直read直到返回EAGAIN（by waiting for an event only after read(2) or write(2) return EAGAIN）。
现在有这样的情况
reciveSize = read(sockfd, buffer, BUF_LEN);
reciveSize小于BUF_LEN的时候，我是可以跳出read的循环了还是必须还要继续read直到出现EAGAIN呢？
理论上返回值小于BUF_LEN不是就已经把缓冲区读完了吗？
如果必须要read到EAGAIN出现，那么为什么呢？
是，返回值小于请求值就可以返回了。

Q: 設置tcp socket option
https://blog.csdn.net/qq_38093301/article/details/105847660

### 日志


### 连调出现的bug
1. 当在浏览器请求http://127.0.0.1:8080/hello等，服务器程序发送完reponse给服务器后，在处理下一个读事件，handleRead时段错误
答: 首先，找到core dump文件。参考:[https://www.pudn.com/news/6292ee86e74b9677e8e0a8c8.html].注意，在生成core dump文件时，即使设置了ulimit -c unlimited，也先查一下ulimit -c 看是否为 unlimited.
    调试core dump， 命令为 gdb ../bin/testHttpServer /var/core/core-testHttpServer-11871-18446744073709551615
    bt查看堆栈信息，不断cotinue
    定位是TcpConnection::handleRead，Buffer::readFd,append, insert出错。
    分析，因为insert会扩容，但writeIndex等

2. 什么时候产生POLLHUP