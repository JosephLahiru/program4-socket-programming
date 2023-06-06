#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_BUFFER_SIZE 1500

struct ThreadArgs {
    int clientSd;
};

void* serviceRequest(void* args) {
    struct ThreadArgs* threadArgs = (struct ThreadArgs*) args;
    int clientSd = threadArgs->clientSd;
    delete threadArgs;  // Clean up the argument struct

    char dataBuf[MAX_BUFFER_SIZE];

    // Step 2: Receive the number of iterations from the client
    int iterations;
    if (recv(clientSd, &iterations, sizeof(iterations), 0) == -1) {
        std::cerr << "recv error" << std::endl;
        close(clientSd);
        pthread_exit(nullptr);
    }

    // Step 3: Read the appropriate number of iterations of data from the client
    int numReads = 0;
    for (int i = 0; i < iterations; ++i) {
        int bytesRead = 0;
        int totalBytesRead = 0;

        while (totalBytesRead < MAX_BUFFER_SIZE) {
            bytesRead = read(clientSd, dataBuf + totalBytesRead, MAX_BUFFER_SIZE - totalBytesRead);
            if (bytesRead == -1) {
                std::cerr << "read error" << std::endl;
                close(clientSd);
                pthread_exit(nullptr);
            } else if (bytesRead == 0) {
                break;
            }
            totalBytesRead += bytesRead;
        }

        numReads++;
    }

    // Step 4: Send the number of read() system calls to the client
    if (send(clientSd, &numReads, sizeof(numReads), 0) == -1) {
        std::cerr << "send error" << std::endl;
        close(clientSd);
        pthread_exit(nullptr);
    }

    // Step 5: Close the connection
    close(clientSd);

    // Step 6: Terminate the thread
    pthread_exit(nullptr);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " port" << std::endl;
        return 1;
    }

    const char* port = argv[1];

    // Step 1: Create a socket and bind it to the given port
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(nullptr, port, &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return 1;
    }

    int serverSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverSd == -1) {
        std::cerr << "socket error" << std::endl;
        return 1;
    }

    const int on = 1;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(int));

    if (bind(serverSd, res->ai_addr, res->ai_addrlen) == -1) {
        std::cerr << "bind error" << std::endl;
        close(serverSd);
        return 1;
    }

    freeaddrinfo(res);

    // Step 2: Listen for incoming connections
    if (listen(serverSd, 5) == -1) {
        std::cerr << "listen error" << std::endl;
        close(serverSd);
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        // Step 3: Accept a client connection
        struct sockaddr_storage clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSd = accept(serverSd, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSd == -1) {
            std::cerr << "accept error" << std::endl;
            close(serverSd);
            return 1;
        }

        // Step 4: Create a thread to service the request
        pthread_t thread;
        struct ThreadArgs* threadArgs = new ThreadArgs;
        threadArgs->clientSd = clientSd;

        if (pthread_create(&thread, nullptr, serviceRequest, (void*)threadArgs) != 0) {
            std::cerr << "pthread_create error" << std::endl;
            close(clientSd);
            close(serverSd);
            return 1;
        }

        // Detach the thread to clean up its resources automatically
        pthread_detach(thread);
    }

    // Step 5: Close the server socket (never reached in this code)
    close(serverSd);

    return 0;
}
