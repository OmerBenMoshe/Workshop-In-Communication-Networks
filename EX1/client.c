//
// Created by omer.benmoshe on 5/29/24.
//

#include <stdio.h>           // Standard input/output library
#include <stdlib.h>          // Standard library for memory allocation, process control, etc.
#include <string.h>          // String handling functions
#include <unistd.h>          // Provides access to POSIX operating system API
#include <arpa/inet.h>       // Definitions for internet operations (e.g., sockaddr_in)
#include <sys/time.h>        // Time structure definitions and functions

#define PORT 8081            // The port number on which the server will connect
#define BUFFER_SIZE 1048576  // Buffer size of 1MB

// Constants for message size calculations
#define BYTE_SIZE 8
#define MEGA_COEFFICIENT (1024 * 1024)
#define MSG_SIZE_INC 2           // Factor by which message size will increase
#define DEFAULT_SERVER_IP "132.65.164.102" // Default IP address of the server
#define ITER_POS_BUFF 5          // Factor for selecting the number of iterations
#define WARMUP_ITER_POS_BUFF 11  // Factor for selecting the number of warmup iterations
#define NUM_OF_MSG_SIZES 21      // Number of different message sizes to test

// Function to get the current time in seconds (as a double)
double get_time_in_seconds() {
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec + time.tv_usec / 1000000.0;
}

int main(int argc, char *argv[]) {
  char *server_ip = DEFAULT_SERVER_IP; // Initialize server IP with the default value
  if (argc == 2) {
    server_ip = argv[1]; // If an IP address is provided as an argument, use it
  }

  int sock = 0;                // Socket file descriptor
  struct sockaddr_in serv_addr; // Structure to hold server address information
  char *buffer = malloc(BUFFER_SIZE); // Allocate memory for the buffer
  double start_time, end_time;  // Variables to store start and end time for throughput measurement

  // Array defining the number of iterations for different message sizes
  int iterations[] = {1000000, 1000000, 25000, 1000, 800};
  // Array defining the number of warmup iterations for different message sizes
  int warmup_iterations[] = {500, 200};

  // Create the client socket
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    return EXIT_FAILURE;
  }

  // Define the server address (IPv4, specific port)
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  // Convert the server IP address from text to binary form
  if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    perror("Invalid address/ Address not supported");
    return EXIT_FAILURE;
  }

  // Connect to the server
  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("Connection Failed");
    return EXIT_FAILURE;
  }

  size_t size = 1; // Start with a message size of 1 byte
  printf("message size throughput units\n");

  // Loop through different message sizes
  for (int i = 0; i < NUM_OF_MSG_SIZES; ++i) {
    memset(buffer, 'a', size); // Fill the buffer with 'a' characters
    size_t bytes_sent = 0;     // Counter for the total bytes sent
    ssize_t bytes_written;     // Number of bytes written in each write call

    int pos = i / ITER_POS_BUFF; // Determine the number of iterations based on message size
    int warmup_pos = i / WARMUP_ITER_POS_BUFF; // Determine the number of warmup iterations

    // Warmup phase: send messages without measuring throughput
    for (int j = 0; j < warmup_iterations[warmup_pos]; ++j) {
      bytes_written = write(sock, buffer, size);
      if (bytes_written < 0) {
        perror("write");
        exit(EXIT_FAILURE);
      }
    }

    bytes_sent = 0; // Reset bytes_sent counter
    start_time = get_time_in_seconds(); // Start time for throughput measurement

    // Send the messages and accumulate the total bytes sent
    for (int j = 0; j < iterations[pos]; ++j) {
      bytes_written = write(sock, buffer, size);
      if (bytes_written < 0) {
        perror("write");
        exit(EXIT_FAILURE);
      }
      bytes_sent += (size_t) bytes_written;
    }
    end_time = get_time_in_seconds(); // End time for throughput measurement

    // Calculate time difference
    double time_diff = end_time - start_time;
    fflush(stdout);
    // Calculate throughput in Mbps
    double throughput = ((double)(bytes_sent * BYTE_SIZE)) / (time_diff * MEGA_COEFFICIENT); // Mbps per unit packet size
    printf("%8zu %11f Mbps\n", size, throughput); // Print the message size and throughput
    size *= MSG_SIZE_INC; // Double the message size for the next iteration
  }

  close(sock); // Close the socket
  free(buffer); // Free the allocated memory
  return EXIT_SUCCESS;
}
