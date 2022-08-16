#pragma once

#include <map>
#include <string>

#include "../Buffer.h"
namespace mytinywebserver {

class HttpResponse {
   public:
    // 响应码
    enum HttpStatusCode {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

    explicit HttpResponse(bool close)
        : statusCode_(kUnknown), closeConnection_(close) {}

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

    void setStatusMessage(const std::string& message) {
        statusMessage_ = message;
    }

    void setCloseConnection(bool on) { closeConnection_ = on; }

    bool closeConnection() const { return closeConnection_; }

    void setContentType(const std::string& contentType) {
        addHeader("Content-Type", contentType);
    }

    // FIXME: replace string with StringPiece
    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    void setBody(const std::string& body) { body_ = body; }
    // 将httpresponse内容序列化输出到buffer
    void appendToBuffer(Buffer* output) const;

   private:
    std::map<std::string, std::string> headers_;
    // FIXME: 响应码和文字用map表示，类似sylar
    HttpStatusCode statusCode_;  // 响应码
    // FIXME: add http version
    // 响应码对应的文字描述 eg： 200 ok 400 not found
    std::string statusMessage_;
    // 是否为短连接，为了序列化输出时，判断headers是否需要包含connection属性
    bool closeConnection_;
    std::string body_;
};

}  // namespace mytinywebserver