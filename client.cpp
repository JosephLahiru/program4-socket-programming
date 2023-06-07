#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sys/uio.h>
#include <netdb.h>
#include <chrono>

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        std::cerr << "Usage: " << argv[0] << " <serverName> <port> <repetition> <nbufs> <bufsize> <type>" << std::endl;
        return 1;
    }

    std::string serverName = argv[1];
    int port = std::stoi(argv[2]);
    int repetition = std::stoi(argv[3]);
    int nbufs = std::stoi(argv[4]);
    int bufsize = std::stoi(argv[5]);
    int type = std::stoi(argv[6]);

    addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(serverName.c_str(), argv[2], &hints, &res) != 0)
    {
        std::cerr << "Error getting server address" << std::endl;
        return 1;
    }

    int client_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_sock < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    if (connect(client_sock, res->ai_addr, res->ai_addrlen) < 0)
    {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    freeaddrinfo(res);

    char databuf[nbufs][bufsize];
    memset(databuf, 'a', nbufs * bufsize);

    send(client_sock, &repetition, sizeof(int), 0);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < repetition; ++i)
    {
        if (type == 1)
        {
            for (int j = 0; j < nbufs; j++)
            {
                write(client_sock, databuf[j], bufsize);
            }
        }
        else if (type == 2)
        {
            struct iovec vector[nbufs];
            for (int j = 0; j < nbufs; j++)
            {
                vector[j].iov_base = databuf[j];
                vector[j].iov_len = bufsize;
            }
            writev(client_sock, vector, nbufs);
        }
        else if (type == 3)
        {
            write(client_sock, databuf, nbufs * bufsize);
        }
        else
        {
            std::cerr << "Invalid test type" << std::endl;
            return 1;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start;

    int totalReadCalls;
    recv(client_sock, &totalReadCalls, sizeof(int), 0);

    double throughput = ((nbufs * bufsize * repetition * 8) / (1024.0 * 1024.0 * 1024.0)) / (elapsed.count() / 1000000.0);

    std::cout << "Test (" << type << "): time = " << elapsed.count() << " usec, "
              << "#reads = " << totalReadCalls << ", throughput = " << throughput << " Gbps" << std::endl;

    close(client_sock);

    return 0;
}