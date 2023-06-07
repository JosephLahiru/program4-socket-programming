#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <pthread.h>

const int BUFSIZE = 1500;

// Struct to hold data for each thread
struct ThreadData
{
    int client_sock;
};

// Function to handle individual client requests in separate threads
void *service_request(void *arg)
{
    // Cast the void pointer back to the ThreadData struct
    ThreadData *data = static_cast<ThreadData *>(arg);
    int client_sock = data->client_sock;
    delete data;

    char data_buf[BUFSIZE];
    int iterations;
    int bytes_read;

    // Receive the number of iterations from the client
    recv(client_sock, &iterations, sizeof(int), 0);

    int total_read_calls = 0;
    for (int i = 0; i < iterations; ++i)
    {
        bytes_read = 0;
        // Read data from the client until the buffer is full
        while (bytes_read < BUFSIZE)
        {
            int result = read(client_sock, data_buf + bytes_read, BUFSIZE - bytes_read);
            if (result <= 0)
            {
                close(client_sock);
                pthread_exit(nullptr);
            }
            bytes_read += result;
            total_read_calls++;
        }
    }

    // Send the total number of read calls back to the client
    send(client_sock, &total_read_calls, sizeof(int), 0);
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

    // Create a server socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Set socket options
    const int on = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));

    // Configure server address structure
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the server socket to the specified port
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    // Listen for incoming client connections
    if (listen(server_sock, 5) < 0)
    {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }

    // Accept client connections and create a new thread for each connection
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

        // Allocate memory for thread data and assign the client socket
        ThreadData *data = new ThreadData;
        data->client_sock = client_sock;

        // Create and detach a new thread to handle the client request
        pthread_t thread_id;
        pthread_create(&thread_id, nullptr, service_request, data);
        pthread_detach(thread_id);
    }

    return 0;
}