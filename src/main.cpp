#include "epoll_server.h"
#include <iostream>

int main() {
  EpollServer server(8080);
  server.start();
    return 0;
}