#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

// Global variables
char my_ip[16];
int my_port;
int server_socket;
pthread_t server_thread;

// Structure to store connections
struct Connection {
    char ip[16];
    int port;
    int socket;
};

struct Connection connections[100];
int num_connections = 0;

// Function to handle incoming messages from a connection
void* handle_client(void* client_socket_ptr) {
    int client_socket = *((int*)client_socket_ptr);
    free(client_socket_ptr);

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    getpeername(client_socket, (struct sockaddr*)&client_addr, &addr_len);

    char buffer[1024];
    while (1) {
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            close(client_socket);
            return NULL;
        }
        buffer[bytes_received] = '\0';
        printf("Message received from %s\n", inet_ntoa(client_addr.sin_addr));
        printf("Sender's Port: %d\n", ntohs(client_addr.sin_port));
        printf("Message: \"%s\"\n", buffer);
    }
}

// Function to display the available user interface options
void display_help() {
    printf("Available commands:\n");
    printf("1. help - Display available commands.\n");
    printf("2. myip - Display your IP address.\n");
    printf("3. myport - Display the port you are listening on.\n");
    printf("4. connect <destination> <port no> - Connect to another peer.\n");
    printf("5. list - Display a numbered list of all connections.\n");
    printf("6. terminate <connection id> - Terminate a connection by its id.\n");
    printf("7. send <connection id> <message> - Send a message to a connection.\n");
    printf("8. exit - Close all connections and terminate the application.\n");
}

// Function to get the local IP address
void get_my_ip() {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getsockname(server_socket, (struct sockaddr*)&addr, &addr_len);
    strcpy(my_ip, inet_ntoa(addr.sin_addr));
    printf("My IP address: %s\n", my_ip);
}

// Function to display the listening port
void get_my_port() {
    printf("My listening port: %d\n", my_port);
}

// Function to establish a new connection to a remote peer
void connect_to_peer(char* destination, int port) {
    for (int i = 0; i < num_connections; i++) {
        if (strcmp(destination, connections[i].ip) == 0 && port == connections[i].port) {
            printf("Error: Self-connection is not allowed.\n");
            return;
        }
    }

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(destination);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        return;
    }

    pthread_t client_thread;
    int* client_socket_ptr = malloc(sizeof(int));
    *client_socket_ptr = client_socket;
    pthread_create(&client_thread, NULL, handle_client, client_socket_ptr);

    strcpy(connections[num_connections].ip, destination);
    connections[num_connections].port = port;
    connections[num_connections].socket = client_socket;
    num_connections++;

    printf("Connected to %s:%d\n", destination, port);
}

// Function to list all connections
void list_connections() {
    printf("List of connections:\n");
    for (int i = 0; i < num_connections; i++) {
        printf("%d: %s %d\n", i + 1, connections[i].ip, connections[i].port);
    }
}

// Function to terminate a connection by its index
void terminate_connection(int index) {
    if (index < 0 || index >= num_connections) {
        printf("Error: Invalid connection index.\n");
        return;
    }

    close(connections[index].socket);

    // Remove the terminated connection from the list
    for (int i = index; i < num_connections - 1; i++) {
        connections[i] = connections[i + 1];
    }
    num_connections--;

    printf("Connection terminated.\n");
}

// Function to send a message to a specific connection
void send_message(int index, char* message) {
    if (index < 0 || index >= num_connections) {
        printf("Error: Invalid connection index.\n");
        return;
    }

    int dest_socket = connections[index].socket;

    if (send(dest_socket, message, strlen(message), 0) == -1) {
        perror("Error sending message");
        return;
    }

    printf("Message sent to %s:%d\n", connections[index].ip, connections[index].port);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    my_port = atoi(argv[1]);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(my_port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        return 1;
    }

    printf("Listening on %s:%d\n", my_ip, my_port);

    char command[100];
    while (1) {
        printf("Enter a command (type 'help' for options): ");
        fgets(command, sizeof(command), stdin);
        command[strlen(command) - 1] = '\0';

        if (strcmp(command, "help") == 0) {
            display_help();
        } else if (strcmp(command, "myip") == 0) {
            get_my_ip();
        } else if (strcmp(command, "myport") == 0) {
            get_my_port();
        } else if (strstr(command, "connect") == command) {
            char destination[16];
            int port;
            if (sscanf(command, "connect %15s %d", destination, &port) == 2) {
                connect_to_peer(destination, port);
            } else {
                printf("Usage: connect <destination> <port no>\n");
            }
        } else if (strcmp(command, "list") == 0) {
            list_connections();
        } else if (strstr(command, "terminate") == command) {
            int index;
            if (sscanf(command, "terminate %d", &index) == 1) {
                terminate_connection(index - 1);
            } else {
                printf("Usage: terminate <connection id>\n");
            }
        } else if (strstr(command, "send") == command) {
            int index;
            char message[1024];
            if (sscanf(command, "send %d %[^\n]", &index, message) == 2) {
                send_message(index - 1, message);
            } else {
                printf("Usage: send <connection id> <message>\n");
            }
        } else if (strcmp(command, "exit") == 0) {
            // Close all connections and exit
            for (int i = 0; i < num_connections; i++) {
                close(connections[i].socket);
            }
            close(server_socket);
            exit(0);
        } else {
            printf("Unknown command. Type 'help' for options.\n");
        }
    }

    close(server_socket);
    return 0;
}
