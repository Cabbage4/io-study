#include <string.h>
#include <netinet/in.h>
#include <unistd.h> // close
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h> //设置非阻塞

using namespace std;

int main()
{
    const int EVENTS_SIZE = 20;
    char buffer[1024];

    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sockAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(11277)};
    sockAddr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(server_fd, (sockaddr *)&sockAddr, sizeof(sockAddr)) == -1)
    {
        cout << "bind error" << endl;
        return -1;
    }

    if (listen(server_fd, 10) == -1)
    {
        cout << "listen error" << endl;
        return -1;
    }

    int epoll_fd = epoll_create(1);

    epoll_event epev{.events = EPOLLIN};
    epev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &epev);

    epoll_event events[EVENTS_SIZE];

    while (true)
    {
        int eNum = epoll_wait(epoll_fd, events, EVENTS_SIZE, -1);

        if (eNum == -1)
        {
            cout << "epoll_wait" << endl;
            return -1;
        }

        for (int i = 0; i < eNum; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                if (events[i].events & EPOLLIN)
                {
                    sockaddr_in cli_addr;
                    socklen_t length = sizeof(cli_addr);
                    int client_fd = accept(server_fd, (sockaddr *)&cli_addr, &length);
                    if (client_fd <= 0)
                    {
                        continue;
                    }

                    epev.events = EPOLLIN | EPOLLET;
                    epev.data.fd = client_fd;
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    if (flags < 0)
                    {
                        cout << "set no block error, fd:" << client_fd << endl;
                        continue;
                    }

                    if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0)
                    {
                        cout << "set no block error, fd:" << client_fd << endl;
                        continue;
                    }

                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &epev);
                }

                continue;
            }

            if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP)
            {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                close(events[i].data.fd);
                continue;
            }

            if (events[i].events & EPOLLIN)
            {

                int len = read(events[i].data.fd, buffer, sizeof(buffer));
                if (len == -1)
                {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                    close(events[i].data.fd);
                    continue;
                }

                cout << buffer << endl;
            }
        }
    }
}