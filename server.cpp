#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <pthread.h>

const int BUFSIZE = 1500;

struct ThreadData
{
    int client_sock;
};

void *service_request(void *arg)
{
    ThreadData *data = static_cast<ThreadData *>(arg);
    int client_sock = data->client_sock;
    delete data;

    char dataBuf[BUFSIZE];
    int iterations;
    int bytesRead;

    recv(client_sock, &iterations, sizeof(int), 0);

    int totalReadCalls = 0;
    for (int i = 0; i < iterations; ++i)
    {
        bytesRead = 0;
        while (bytesRead < BUFSIZE)
        {
            int result = read(client_sock, dataBuf + bytesRead, BUFSIZE - bytesRead);
            if (result <= 0)
            {
                close(client_sock);
                pthread_exit(nullptr);
            }
            bytesRead += result;
            totalReadCalls++;
        }
    }

    send(client_sock, &totalReadCalls, sizeof(int), 0);
    close(client_sock);
    pthread_exit(nullptr);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    const int on = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    if (listen(server_sock, 5) < 0)
    {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr *)&client_addr, &client_addr_len);

        if (client_sock < 0)
        {
            std::cerr << "Error accepting client connection" << std::endl;
            continue;
        }

        ThreadData *data = new ThreadData;
        data->client_sock = client_sock;

        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, service_request, data);
        pthread_detach(thread_id);
    }

    return 0;
}