#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>

using namespace std;

const int MAX_FD = 10;

int main()
{
    cout << "server start" << endl;

    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(11277)};
    addr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(server_fd, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cout << "bind error";
        return -1;
    }

    if (listen(server_fd, 10) < 0)
    {
        cout << "linsten error";
        return -1;
    }

    int epoll_fd = epoll_create(MAX_FD);
    epoll_event ee;
    ee.events = EPOLLIN;
    ee.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ee);

    char buffer[1024];
    epoll_event eeList[MAX_FD];
    while (true)
    {
        int event_num = epoll_wait(epoll_fd, eeList, MAX_FD, -1);

        for (int i = 0; i < event_num; i++)
        {
            if (eeList[i].data.fd == server_fd)
            {
                sockaddr_in cli_addr;
                socklen_t length = sizeof(cli_addr);
                int client_fd = accept(server_fd, (sockaddr *)&cli_addr, &length);
                ee.events = EPOLLIN;
                ee.data.fd = client_fd;

                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ee);
                continue;
            }

            int len = read(eeList[i].data.fd, buffer, sizeof(buffer));
            if (len <= 0)
            {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, eeList[i].data.fd, NULL);
                close(eeList[i].data.fd);
            }
            else
            {
                cout << buffer << endl;
                memset(buffer, 0, sizeof(buffer));

                string data = "hello world";
                string rsp = "HTTP/1.1 200 ok\r\nDate: Fri, 22 May 2009 06:07:21 GMT\r\nContent-Type: text\r\n\r\n" + data;
                write(eeList[i].data.fd, (void *)rsp.c_str(), rsp.size());

                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, eeList[i].data.fd, NULL);
                close(eeList[i].data.fd);
            }
        }
    }
}