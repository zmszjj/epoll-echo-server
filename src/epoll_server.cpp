#include "epoll_server.h"   // 包含我们刚才写的类图纸
#include <asm-generic/socket.h>
#include <cstdio>
#include <iostream>           // 用于 std::cout 打印输出
#include <cstring>            // 用于 memset 清空内存
#include <netinet/in.h>
#include <ostream>
#include <unistd.h>           // 用于 read, write, close 等系统调用
#include <fcntl.h>            // 用于 fcntl 设置非阻塞
#include <arpa/inet.h>        // 用于 htons, inet_addr (网络字节序转换)
#include <sys/socket.h>       // 用于 socket, bind, listen, accept
#include <sys/epoll.h>        // 用于 epoll_create1, epoll_ctl, epoll_wait 核心函数
#include <cerrno>             // 用于 errno (获取系统错误码)
EpollServer::EpollServer(int port) : port_(port),listen_fd(-1),epoll_fd_(-1){}
void EpollServer::set_noblocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags==-1) flags = 0;
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    
}
void EpollServer::start(){
    listen_fd=socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port_);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind ERROR"); // 如果报错，会打印出具体原因（比如 Address already in use）
        exit(-1);             // 退出程序
    }
      if (listen(listen_fd, 128) < 0) {
        perror("listen ERROR");
        exit(-1);
    }
    set_noblocking(listen_fd);

    epoll_fd_=epoll_create1(0);

    if(epoll_fd_==-1) {
    perror("epoll_fd_ ERROR");
    return;
    }
    struct epoll_event ev,events[1024];
    ev.events = EPOLLIN;
    ev.data.fd = listen_fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd, &ev);
    std::cout <<"Epoll服务器启动,正在监听端口: " << port_ << std::endl;
    char buff[1024];
    while (true) {
        int n = epoll_wait(epoll_fd_,events, 1024, -1);
        for(int i=0;i<n;i++)
        {int current_fd = events[i].data.fd;
        if (current_fd == listen_fd) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) continue;
            std::cout << "新客户接入,fd:" << client_fd << std::endl;
            set_noblocking(client_fd);
            ev.events=EPOLLIN;
            ev.data.fd=client_fd;
            epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev);
        }
        else {
            memset(buff, 0, sizeof(buff));
            int len = read(current_fd,buff, sizeof(buff) - 1);
            if (len > 0) {
                std::cout << "收到来自fd" << current_fd << "的消息:" << buff << std::endl;
                write(current_fd, buff, len);          
            }
            else if (len == 0) {
            std::cout << "服务器断开,fd:" << current_fd << std::endl;
            epoll_ctl(epoll_fd_, EPOLL_CTL_DEL,current_fd, nullptr);
            close(current_fd);
            }
            else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read ERROR");
            epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, current_fd, nullptr);
            close(current_fd);
            }
            }
        }
        }


    
    }


}


