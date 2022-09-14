#include <netinet/in.h>
#include <string.h> // memset
#include <unistd.h> // close
#include <iostream> // cout cerr endl

int main()
{
    char buffer[1024];

    int server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        std::cerr << "create failure" << std::endl;
        return -1;
    }

    sockaddr_in server_socket_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(11277)};
    server_socket_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_fd, (const sockaddr *)&server_socket_addr, sizeof(server_socket_addr)) < 0)
    {
        std::cerr << "bind failure" << std::endl;
        return -1;
    }

    if (listen(server_socket_fd, 7) < 0)
    {
        std::cerr << "listen failure" << std::endl;
        return -1;
    }

    fd_set server_fd_set;
    FD_ZERO(&server_fd_set);
    FD_SET(server_socket_fd, &server_fd_set);

    int max_fd = server_socket_fd;

    while (true)
    {
        select(max_fd + 1, &server_fd_set, NULL, NULL, NULL);

        for (int i = 0; i < max_fd + 1; i++)
        {
            if (!FD_ISSET(i, &server_fd_set))
            {
                continue;
            }

            if (i == server_socket_fd)
            {
                sockaddr_in client_addr;
                socklen_t address_len;
                int client_sock_fd = accept(server_socket_fd, (sockaddr *)&client_addr, &address_len);
                FD_SET(client_sock_fd, &server_fd_set);

                max_fd = std::max(client_sock_fd, max_fd);

                continue;
            }

            memset(buffer, 0, sizeof(buffer));
            int str_len = recv(i, buffer, sizeof(buffer), 0);
            std::cout << buffer << std::endl;

            if (str_len == 0)
            {
                FD_CLR(i, &server_fd_set);
                close(i);
            }
        }
    }

    close(server_socket_fd);

    return 0;
}