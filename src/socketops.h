// Copyright©2022 Erin
#pragma once

namespace mytinywebserver {
namespace socket {

int createSocketFd();
void closeFd(int fd);

}  // namespace socket
}  // namespace mytinywebserver