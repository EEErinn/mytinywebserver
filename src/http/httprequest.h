#pragma once

#include <map>
#include <string>
#include <utility>

#include "../Timestamp.h"

namespace mytinywebserver {

class HttpRequest {
   public:
    // 请求方法
    enum Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };
    // http版本
    enum Version { kUnknown, kHttp10, kHttp11 };

    HttpRequest() : method_(kInvalid), version_(kUnknown) {}

    // set and get
    bool setMethod(const char* start, const char* end) {
        assert(method_ == kInvalid);
        std::string m(start, end);
        if (m == "GET") {
            method_ = kGet;
        } else if (m == "POST") {
            method_ = kPost;
        } else if (m == "HEAD") {
            method_ = kHead;
        } else if (m == "PUT") {
            method_ = kPut;
        } else if (m == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    Method method() const { return method_; }

    void setVersion(Version v) { version_ = v; }
    Version getVersion() const { return version_; }

    void setPath(const char* start, const char* end) {
        path_.assign(start, end);
    }
    const std::string& path() const { return path_; }

    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }
    const std::string& query() const { return query_; }

    void setReceiveTime(Timestamp t) { receiveTime_ = t; }
    Timestamp receiveTime() const { return receiveTime_; }

    // 字符串形式返回方法名称
    const char* methodString() const {
        const char* result = "UNKNOWN";
        switch (method_) {
            case kGet:
                result = "GET";
                break;
            case kPost:
                result = "POST";
                break;
            case kHead:
                result = "HEAD";
                break;
            case kPut:
                result = "PUT";
                break;
            case kDelete:
                result = "DELETE";
                break;
            default:
                break;
        }
        return result;
    }

    void setHeader(const std::string& key, const std::string& val) {
        headers_[key] = val;
    }

    bool hasHeader(const std::string& key) {
        auto i = headers_.find(key);
        return i != headers_.end();
    }

    std::string getHeader(const std::string& field) const {
        std::string result;
        std::map<std::string, std::string>::const_iterator it =
            headers_.find(field);
        if (it != headers_.end()) {
            result = it->second;
        }
        return result;
    }

    const std::map<std::string, std::string>& headers() const {
        return headers_;
    }

    void swap(HttpRequest& that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        // receiveTime_.swap(that.receiveTime_); //FIXME
        headers_.swap(that.headers_);
    }

   private:
    Method method_;      // GET
    Version version_;    // HTTP 1.1
    std::string path_;   // path /PATH/XX
    std::string query_;  // param id=10&v=20
    // string m_fragment
    // string m_body
    Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;
};

}  // namespace mytinywebserver