#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H
class EpollServer {
    public:
    EpollServer(int port);
    void start();
    private:
    int port_;
    int listen_fd;
    int epoll_fd_;
    void set_noblocking(int fd);
};
#endif