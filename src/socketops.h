// Copyright©2022 Erin
#pragma once

namespace mytinywebserver {
namespace socket {

int createSocketFd();
void closeFd(int fd);
int getSocketError(int sockfd);

}  // namespace socket
}  // namespace mytinywebserver