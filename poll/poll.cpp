
#include <string.h>
#include <netinet/in.h>
#include <unistd.h> // close
#include <poll.h>   //pollfd
#include <iostream>
#include <vector>

int main()
{
    char buffer[1024];

    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        std::cerr << "create failure" << std::endl;
        return -1;
    }

    sockaddr_in addr_in = {
        .sin_family = AF_INET,
        .sin_port = htons(11277)};
    addr_in.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_fd, (sockaddr *)(&addr_in), sizeof(sockaddr_in)) < 0)
    {
        std::cerr << "bind failure" << std::endl;
        return -1;
    }

    if (listen(server_socket_fd, 7) < 0)
    {
        std::cerr << "listen failure" << std::endl;
        return -1;
    }

    std::vector<pollfd> server_fd_list;
    server_fd_list.push_back(pollfd{.fd = server_socket_fd, .events = POLLIN});

    while (true)
    {
        pollfd fd_list[server_fd_list.size()];
        int fd_size = server_fd_list.size();
        memcpy(fd_list, &server_fd_list[0], fd_size * sizeof(pollfd)); // 此处有内存泄漏，仅做学习使用

        poll(fd_list, fd_size, -1);

        for (int i = 0; i < fd_size; i++)
        {
            if (!(POLLIN & fd_list[i].revents))
            {
                continue;
            }

            if (fd_list[i].fd == server_socket_fd)
            {
                struct sockaddr_in client_addr;
                socklen_t address_len;
                int client_sock_fd = accept(server_socket_fd, (sockaddr *)&client_addr, &address_len);
                server_fd_list.push_back(pollfd{.fd = client_sock_fd, .events = POLLIN});

                continue;
            }

            memset(buffer, 0, sizeof(buffer));
            int str_len = recv(fd_list[i].fd, buffer, sizeof(buffer), 0);
            std::cout << buffer << std::endl;

            if (str_len == 0)
            {
                server_fd_list.erase(server_fd_list.begin() + i); // 此处有内存泄漏，仅做学习使用
                close(i);
            }
        }
    }

    close(server_socket_fd);
    return 0;
}