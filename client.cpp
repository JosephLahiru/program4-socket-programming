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
    // Check for correct number of arguments
    if (argc != 7)
    {
        std::cerr << "Usage: " << argv[0] << " <serverName> <port> <repetition> <nbufs> <bufsize> <type>" << std::endl;
        return 1;
    }

    // Parse command line arguments
    std::string serverName = argv[1];
    int port = std::stoi(argv[2]);
    int repetition = std::stoi(argv[3]);
    int nbufs = std::stoi(argv[4]);
    int bufsize = std::stoi(argv[5]);
    int type = std::stoi(argv[6]);

    // Set up hints and server address structure
    addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Get server address
    if (getaddrinfo(serverName.c_str(), argv[2], &hints, &res) != 0)
    {
        std::cerr << "Error getting server address" << std::endl;
        return 1;
    }

    // Create client socket
    int client_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (client_sock < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Connect to server
    if (connect(client_sock, res->ai_addr, res->ai_addrlen) < 0)
    {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    // Free server address info
    freeaddrinfo(res);

    // Initialize data buffer
    char databuf[nbufs][bufsize];
    memset(databuf, 'a', nbufs * bufsize);

    // Send repetition count to server
    send(client_sock, &repetition, sizeof(int), 0);

    // Start timer
    auto start = std::chrono::high_resolution_clock::now();

    // Perform the test based on the specified type
    for (int i = 0; i < repetition; ++i)
    {
        if (type == 1)
        {
            // Send data using multiple write() calls
            for (int j = 0; j < nbufs; j++)
            {
                write(client_sock, databuf[j], bufsize);
            }
        }
        else if (type == 2)
        {
            // Send data using a single writev() call with a vector of buffers
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
            // Send data using a single write() call with a large buffer
            write(client_sock, databuf, nbufs * bufsize);
        }
        else
        {
            std::cerr << "Invalid test type" << std::endl;
            return 1;
        }
    }

    // Stop timer
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start;

    // Receive total number of read calls from server
    int totalReadCalls;
    recv(client_sock, &totalReadCalls, sizeof(int), 0);

    // Calculate and display throughput
    double throughput = ((nbufs * bufsize * repetition * 8) / (1024.0 * 1024.0 * 1024.0)) / (elapsed.count() / 1000000.0);
    std::cout << "Test (" << type << "): time = " << elapsed.count() << " usec, "
              << "#reads = " << totalReadCalls << ", throughput = " << throughput << " Gbps" << std::endl;

    // Close client socket
    close(client_sock);

    return 0;
}