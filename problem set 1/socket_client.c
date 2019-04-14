#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<strings.h>
#include<netdb.h>
#include<netinet/in.h>


#define SERVER_ADDRESS "localhost"
#define SERVER_PORT 8888

int main(int argc, char **agv){   
    int socket_fd =0;
    struct sockaddr_in server_socket_addr;

  // Converts localhost into 0.0.0.0
	struct hostent *he = gethostbyname(SERVER_ADDRESS);
	unsigned long server_addr_nbo = *(unsigned long *)(he->h_addr_list[0]);

  // Create socket (IPv4, stream-based, protocol likely set to TCP)
  if (0 > (socket_fd = socket(AF_INET, SOCK_STREAM, 0))) {
    fprintf(stderr, "client failed to create socket\n");
    exit(1);
  }

  // Configure server socket address structure (init to zero, IPv4,
  // network byte order for port and address) 
  bzero(&server_socket_addr, sizeof(server_socket_addr));
  server_socket_addr.sin_family = AF_INET;
  server_socket_addr.sin_port = htons(SERVER_PORT);
  server_socket_addr.sin_addr.s_addr = server_addr_nbo;

  // Connect socket to server
  if (0 > connect(socket_fd, (struct sockaddr *)&server_socket_addr, sizeof(server_socket_addr))) {
    fprintf(stderr, "client failed to connect to %s:%d!\n", SERVER_ADDRESS, SERVER_PORT);
    close(socket_fd);
    exit(1);
  } else {
  	fprintf(stdout, "client connected to to %s:%d!\n", SERVER_ADDRESS, SERVER_PORT);
  }

  // Close the socket and return
  close(socket_fd);
  return 0; 
}

