# Documentation for Network Data Transfer Application

## Overview

This network data transfer application consists of a client-server pair, implemented in C++, that allows transferring data between two nodes in a network. The client sends data to the server using different test types and buffer size combinations, allowing users to evaluate the performance of various data transfer methods.

## Components

### Server

The server listens for incoming connections and receives data from the client. It processes the received data according to the test type specified by the client and sends back the results to the client.

### Client

The client connects to the server and sends data using different test types and buffer size combinations. It measures the throughput of the data transfer and prints the results.

## Test Types

- Test Type 1: The client sends data using multiple write() system calls for each data buffer.
- Test Type 2: The client sends all data buffers at once using writev() with an array of iovec data structures.
- Test Type 3: The client sends all data buffers at once using a single write() call.

## Use Cases

### Performance Evaluation

One of the primary use cases for this application is to evaluate the performance of different data transfer methods in a network environment. Users can run tests with various test types and buffer size combinations to determine the most efficient method for their specific scenario.

### Network Troubleshooting

The application can also be used for network troubleshooting by identifying bottlenecks or issues in the data transfer process. Users can run tests with different parameters to diagnose potential problems and optimize their network configuration accordingly.

### Benchmarking

The application can serve as a benchmarking tool for comparing the performance of different network configurations, hardware, or software. Users can run tests with consistent parameters across different setups to determine which configurations yield the best performance.

## Usage

- Compile both server.cpp and client.cpp on two different nodes in a network (e.g., csslab1 and csslab2).

- Run the server on one node (e.g., csslab1) with your chosen port number:
  `./server <port>`
- Run the client on the other node (e.g., csslab2) with the server's name, port number, and the different test parameters:
  `./client csslab1 <port> <repetition_count> <nbufs> <bufsize> <test_type>`
- Note down the throughput (in Gbps) for each test, as printed by the client. Analyze the results to determine the most efficient data transfer method for your specific scenario.

## Performance Tests

  - Test 1: nbufs = 15, bufsize = 100
    - Type 1: ./client <serverName> <port> 20000 15 100 1
    - Type 2: ./client <serverName> <port> 20000 15 100 2
    - Type 3: ./client <serverName> <port> 20000 15 100 3
  - Test 2: nbufs = 30, bufsize = 50
    - Type 1: ./client <serverName> <port> 20000 30 50 1
    - Type 2: ./client <serverName> <port> 20000 30 50 2
    - Type 3: ./client <serverName> <port> 20000 30 50 3
  - Test 3: nbufs = 60, bufsize = 25
    - Type 1: ./client <serverName> <port> 20000 60 25 1
    - Type 2: ./client <serverName> <port> 20000 60 25 2
    - Type 3: ./client <serverName> <port> 20000 60 25 3
  - Test 4: nbufs = 100, bufsize = 15
    - Type 1: ./client <serverName> <port> 20000 100 15 1
    - Type 2: ./client <serverName> <port> 20000 100 15 2
    - Type 3: ./client <serverName> <port> 20000 100 15 3
