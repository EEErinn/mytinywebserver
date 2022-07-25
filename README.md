# mytinywebserver

## 目录结构
bin: 可执行文件
build: cmake的产物
src: 源代码
lib: 链接文件
tests: 测试文件
CMakeLists.txt: cmake文件

## 开发步骤
*基本功能(尽量8月1号之前)
- 实现事件循环机制，完成事件处理 即eventloop、poller、channel
- 完成事件注册 即Acceptor、socket、InetAddress
- 完成tcp连接 即tcpserver、tcpconnection、buffer
- 多线程 即thread、eventthread、eventthreadpool
- 支持http1.1 url 即httpserver、ttpcontext、httprequest、httpresponse
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
1. 完成事件处理，eventloop - poller - channel

Todos:

2. 完成事件注册 即Acceptor、socket、InetAddress


## 事件处理遇到的问题
> Q: muduo为什么poller需要拥有eventloop? 
- A: 为了在poller中检查当前线程是否是创建所属eventloop的线程。目前认为不重要。因为poller只被eventloop拥有，而eventloop的函数基本保证时线程安全的，所以本人在poller中没有增加eventloop成员变量

> Q: eventloop - poller - channel怎么保证线程安全？
- A：代填坑

> Q: bug: 在testEvent中，在每个线程中使用std::cout打印Cureent::tid(),但无法显示输出。
- A: 通过gdb调试，b 38 b 25 r n n， 可知代码逻辑正确。但直接执行testEvent则不显示, 定位是std::cout的原因。经分析，是epoll_wait阻塞，因此在每次std::cout时都强制从缓冲区取出数据。std::cout.flush()。