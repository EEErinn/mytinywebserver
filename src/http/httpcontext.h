#pragma once

#include <boost/optional.hpp>

#include "httprequest.h"

namespace mytinywebserver {

class Buffer;
class Timestamp;

class HttpContext {
   public:
    enum HttpRequestParseCode {
        NO_REQUEST,   // 请求不完整，需要等待数据传过来
        GET_REQUEST,  // 请求解析完成
        BAD_REQUEST,  // 请求解析错误
    };

    enum HttpRequestParseState {
        kExpectRequestLine,  // 期待请求行
        kExpectHeaders,      // 请求头
        kExpectBody,         // 请求体
        kGotAll,             // 获得所有请求内容
    };
    // note: 没有初始化HttpRequest， c++会自动调用默认构造函数吗

    HttpContext() : state_(kExpectRequestLine) {}

    // 解析请求，注册request_信息
    HttpRequestParseCode parseRequest(Buffer* buf, Timestamp receiveTime);

    // 是否解析请求的所有内容
    bool gotAll() const { return state_ == kGotAll; }

    void reset() {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    const HttpRequest& request() const { return request_; }
    HttpRequest& request() { return request_; }

   private:
    // 被parseRequest调用
    bool processRequestLine(const char* begin, const char* end);
    boost::optional<std::pair<std::string, std::string>> processHeaderLine(
        const char* begin, const char* end);

    HttpRequestParseState state_;  //当前解析状态
    HttpRequest request_;          // 请求体
};

}  // namespace mytinywebserver