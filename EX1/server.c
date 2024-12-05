//
// Created by omer.benmoshe on 5/29/24.
//

#include <stdio.h>           // Standard input/output library
#include <stdlib.h>          // Standard library for memory allocation, process control, etc.
#include <string.h>          // String handling functions
#include <unistd.h>          // Provides access to POSIX operating system API
#include <arpa/inet.h>       // Definitions for internet operations (e.g., sockaddr_in)
#include <sys/time.h>        // Time structure definitions and functions

#define PORT 8081            // The port number on which the server will listen
#define BUFFER_SIZE 1048576  // Buffer size of 1MB

// Function to get the current time in seconds (as a double)
double get_time_in_seconds() {
  struct timeval time;
  gettimeofday(&time, NULL);
  return time.tv_sec + time.tv_usec / 1000000.0;
}

int main() {
  int server_fd, new_socket;   // File descriptors for the server and client sockets
  struct sockaddr_in address;  // Structure to hold server address information
  int opt = 1;                 // Option for setsockopt to allow address reuse
  int addrlen = sizeof(address); // Length of the address structure
  char *buffer = malloc(BUFFER_SIZE); // Allocate memory for the buffer

  // Create the server socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Set socket options to allow address and port reuse
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // Define the server address (IPv4, bind to any available interface, specific port)
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // Bind the server socket to the specified address and port
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Set the socket to listen for incoming connections (with a backlog of 3)
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // Accept an incoming connection and create a new socket for communication
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  printf("Connected to client\n");

  // Loop to continuously read data from the client until the connection is closed
  while (1) {
    ssize_t bytes_read;
    while ((bytes_read = read(new_socket, buffer, BUFFER_SIZE)) > 0) {}
    if (bytes_read < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    } else if (bytes_read == 0) {
      break;  // Exit the loop if the client closes the connection
    }
  }

  printf("ended\n");

  // Clean up: close the sockets and free the allocated memory
  close(new_socket);
  close(server_fd);
  free(buffer);
  return 0;
}
