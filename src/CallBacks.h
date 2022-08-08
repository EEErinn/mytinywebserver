#pragma once

#include <functional>
#include <memory>

namespace mytinywebserver {
class TcpConnection;
class Buffer;
class Timestamp;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallBack_ = std::function<void(const TcpConnectionPtr&)>;
using CloseCallBack_ = std::function<void(const TcpConnectionPtr&)>;

using MessageCallBack_ =
    std::function<void(TcpConnectionPtr, Buffer*, Timestamp)>;

}  // namespace mytinywebserver
