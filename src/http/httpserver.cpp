#include "httpserver.h"

#include "../log/LogUtils.h"

namespace mytinywebserver {

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr,
                       const std::string& name, int numThreads)
    : m_server(loop, listenAddr, numThreads, name) {
    m_server.setConnectionCallBack(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    m_server.setMessageCallBack(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
}

HttpServer::~HttpServer() {
    LOG_DEBUG << "HttpServer::~HttpServer[" << m_server.getName()
              << "] destructing ";
}

void HttpServer::start() {
    LOG_INFO << "HttpServer[" << m_server.getName() << "] starts listening on "
             << m_server.getIpPort();
    m_server.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setContext(HttpContext());  // 设置httpContext
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp receiveTime) {
    // 将conn中的boost::any 的 context准换成HttpContext
    HttpContext* context =
        boost::any_cast<HttpContext>(conn->getMutableContext());
    HttpContext::HttpRequestParseCode res =
        context->parseRequest(buf, receiveTime);
    if (res == HttpContext::HttpRequestParseCode::BAD_REQUEST) {
        LOG_DEBUG << "HTTP/1.1 400 Bad Request";
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    } else if (res == HttpContext::HttpRequestParseCode::GET_REQUEST) {
        LOG_DEBUG << "GET_REQUEST";
        LOG_DEBUG << "buf extra data : " << buf->readableSize();
        // LOG_DEBUG << buf->retrieveAllAsString();
        onRequest(conn, context->request());
        context->reset();  // 重置上下文，将request置空
    } else if (res == HttpContext::HttpRequestParseCode::NO_REQUEST) {
        LOG_DEBUG << "NO_REQUEST";
    }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn,
                           const HttpRequest& req) {
    const std::string& connection = req.getHeader("Connection");
    // true 为 短连接，需要服务端关闭连接（可能会有大量time_wait）；
    // false 为 长连接。
    bool close =
        (connection == "close") || (req.getVersion() == HttpRequest::kHttp10 &&
                                    connection != "Keep-Alive");
    HttpResponse response(close);
    // 业务逻辑：根据连接的request内容，返回response
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);  // 发送response
    if (response.closeConnection()) {
        conn->shutdown();
    }
}

}  // namespace mytinywebserver