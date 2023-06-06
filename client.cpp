#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <chrono>
#include <vector>

#define MAX_BUFFER_SIZE 1500

void performMultipleWrites(int clientSd, int nbufs, int bufsize)
{
    char databuf[nbufs][bufsize];
    for (int i = 0; i < nbufs; ++i)
    {
        // Fill the data buffer with some data
        // You can modify this part according to your specific requirements
        memset(databuf[i], 'A' + (i % 26), bufsize);
    }

    for (int j = 0; j < nbufs; ++j)
    {
        if (write(clientSd, databuf[j], bufsize) == -1)
        {
            std::cerr << "write error" << std::endl;
            close(clientSd);
            exit(1);
        }
    }
}

void performWritev(int clientSd, int nbufs, int bufsize)
{
    char databuf[nbufs][bufsize];
    struct iovec vector[nbufs];
    for (int i = 0; i < nbufs; ++i)
    {
        // Fill the data buffer with some data
        // You can modify this part according to your specific requirements
        memset(databuf[i], 'A' + (i % 26), bufsize);

        vector[i].iov_base = databuf[i];
        vector[i].iov_len = bufsize;
    }

    if (write(clientSd, vector, nbufs) == -1)
    {
        std::cerr << "writev error" << std::endl;
        close(clientSd);
        exit(1);
    }
}

void performSingleWrite(int clientSd, int nbufs, int bufsize)
{
    char databuf[nbufs][bufsize];
    std::vector<char> combinedBuf(nbufs * bufsize);

    for (int i = 0; i < nbufs; ++i)
    {
        // Fill the data buffer with some data
        // You can modify this part according to your specific requirements
        memset(databuf[i], 'A' + (i % 26), bufsize);
        memcpy(combinedBuf.data() + i * bufsize, databuf[i], bufsize);
    }

    if (write(clientSd, combinedBuf.data(), nbufs * bufsize) == -1)
    {
        std::cerr << "write error" << std::endl;
        close(clientSd);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        std::cerr << "Usage: " << argv[0] << " serverName port repetition nbufs bufsize type" << std::endl;
        return 1;
    }

    const char *serverName = argv[1];
    const char *port = argv[2];
    int repetition = std::stoi(argv[3]);
    int nbufs = std::stoi(argv[4]);
    int bufsize = std::stoi(argv[5]);
    int type = std::stoi(argv[6]);

    // Step 1: Establish a connection to the server
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(serverName, port, &hints, &res);
    if (status != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return 1;
    }

    int clientSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (clientSd == -1)
    {
        std::cerr << "socket error" << std::endl;
        return 1;
    }

    if (connect(clientSd, res->ai_addr, res->ai_addrlen) == -1)
    {
        std::cerr << "connect error" << std::endl;
        close(clientSd);
        return 1;
    }

    freeaddrinfo(res);

    std::cout << "Connected to the server." << std::endl;

    // Step 2: Send the number of iterations to the server
    int iterations = repetition * nbufs;
    if (send(clientSd, &iterations, sizeof(iterations), 0) == -1)
    {
        std::cerr << "send error" << std::endl;
        close(clientSd);
        return 1;
    }

    // Step 3: Perform the appropriate number of tests with the server
    // Measure the time it takes for the tests using chrono library
    auto startTime = std::chrono::high_resolution_clock::now();

    switch (type)
    {
    case 1:
        for (int i = 0; i < repetition; ++i)
        {
            performMultipleWrites(clientSd, nbufs, bufsize);
        }
        break;
    case 2:
        for (int i = 0; i < repetition; ++i)
        {
            performWritev(clientSd, nbufs, bufsize);
        }
        break;
    case 3:
        for (int i = 0; i < repetition; ++i)
        {
            performSingleWrite(clientSd, nbufs, bufsize);
        }
        break;
    default:
        std::cerr << "Invalid test type" << std::endl;
        close(clientSd);
        return 1;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

    // Step 4: Receive the number of read() system calls from the server
    int numReads;
    if (recv(clientSd, &numReads, sizeof(numReads), 0) == -1)
    {
        std::cerr << "recv error" << std::endl;
        close(clientSd);
        return 1;
    }

    // Step 5: Print information about the test
    double throughput = (nbufs * bufsize * 8.0 * iterations) / (duration.count() / 1000000.0); // throughput in Gbps

    std::cout << "Test " << type << ": time = " << duration.count() << " usec, #reads = " << numReads
              << ", throughput " << throughput << " Gbps" << std::endl;

    // Step 6: Close the socket
    close(clientSd);

    return 0;
}
