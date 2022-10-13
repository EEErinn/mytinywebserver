#pragma once
#include <memory>
#include <string>

#include "../Buffer.h"
#include "../InetAddress.h"
#include "../Tcpserver.h"
#include "../Timestamp.h"
#include "../eventloop.h"
#include "httpcontext.h"
#include "httprequest.h"
#include "httpresponse.h"

namespace mytinywebserver {

class HttpServer {
   public:
    // 上层业务逻辑：处理请求体request，返回response。该回调函数被OnRequset调用
    using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop, const InetAddress& listenAddr,
               const std::string& name, int numThreads);

    ~HttpServer();

    /// Not thread safe, callback be registered before calling start().
    void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

    void start();

   private:
    // 给conn设置httpcontext
    void onConnection(const TcpConnectionPtr& conn);
    // 有数据读入时，tcpconnection handleRead
    // 将数据读入inputBuffer，然后调用messageCallback_
    //  业务逻辑处理: 解析，计算，发送
    void onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                   Timestamp receiveTime);
    // 被onMessage调用。根据连接和request得到response，并发送给客户端
    void onRequest(const TcpConnectionPtr&, const HttpRequest&);

    void delayTimer(const TcpConnectionPtr& conn);

    TcpServer m_server;
    HttpCallback httpCallback_;
};

}  // namespace mytinywebserver