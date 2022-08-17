#include "httpcontext.h"

#include <iostream>

#include "../Buffer.h"
#include "../Timestamp.h"

namespace mytinywebserver {

// bool checkUrl(const char* start, const char* end) {
//     if (*end != ' ' || end - start <= 7) return false;

//     if (std::equal(start, start + 7, "http://")) {
//         start += 7;
//         const char* path = std::find(start, end, '/');
//         if (path != end) {
//             return true;
//         }
//     }
//     return false;
// }

bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');  //找第一个空格
    // start，space之间是 Method
    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;
        // start, space之间是 url
        space = std::find(start, end, ' ');  // 找第二个空格
        if (space != end) {
            // 检查url格式
            // if(!checkUrl(start, space)) return false;
            const char* question =
                std::find(start, space, '?');  // 找？ 看有没有参数
            if (question != space)             // 有？ 有参数
            {
                request_.setPath(start, question);   // ？前是路径
                request_.setQuery(question, space);  // ？后是参数
            } else                                   // 没有参数只有路径
            {
                request_.setPath(start, space);  //
            }
            start = space + 1;
            // 最后 分析协议版本 是不是8个字节，是不是为HTTP/1.
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                } else if (*(end - 1) == '0') {
                    request_.setVersion(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

boost::optional<std::pair<std::string, std::string>>
HttpContext::processHeaderLine(const char* start, const char* end) {
    const char* colon = std::find(start, end, ':');
    if (colon != end) {
        std::string key(start, colon);
        ++colon;
        while (colon < end && isspace(*colon)) {
            ++colon;
        }
        std::string value(colon, end);
        // FIXME:
        while (!value.empty() && isspace(value[value.size() - 1])) {
            value.resize(value.size() - 1);
        }
        return boost::make_optional<std::pair<std::string, std::string>>(
            {key, value});
    }
    return {};
}

HttpContext::HttpRequestParseCode HttpContext::parseRequest(
    Buffer* buf, Timestamp receiveTime) {
    const char* crlf;
    // 找到\r\n，说明有完整的一行，进入循环解析完整的一行
    while ((crlf = buf->findCRLF())) {
        switch (state_) {
            case kExpectRequestLine: {
                bool suceed =
                    processRequestLine(buf->beginRead(), crlf);  //解析请求行
                if (suceed) {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(
                        crlf + 2);  // 解析完移动buffer指针到下一个起始位置
                    state_ = kExpectHeaders;
                    break;
                }
                return BAD_REQUEST;
            }
            case kExpectHeaders: {
                auto ops = processHeaderLine(buf->beginRead(), crlf);
                buf->retrieveUntil(
                    crlf + 2);  // note 即使结尾也要移2 不然bufer里始终保留\r\n
                                // 让下一次请求失效
                if (ops) {
                    const std::string key = ops.get().first;
                    const std::string val = ops.get().second;
                    request_.setHeader(key, val);
                    if (request_.hasHeader("Content-length")) {
                        state_ = kExpectBody;
                    }
                } else {
                    // FIXME: empty line, end of header
                    return GET_REQUEST;
                }
                break;
            }
            case kExpectBody: {
                // FIXME: 解析Body
                return GET_REQUEST;
                break;
            }
            default: {
                return BAD_REQUEST;
            }
        }
    }
    return NO_REQUEST;
}

}  // namespace mytinywebserver
