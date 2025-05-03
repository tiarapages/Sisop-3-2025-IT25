#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>


#define PORT 8080
#define BUFFER_SIZE 1024
#define DATABASE_DIR "server/database"
#define LOG_FILE "server/server.log"

void log_message(const char *source, const char *action, const char *info) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);

    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm_info);

    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, timestamp, action, info);
    fclose(log_file);
}

void handle_sigint(int sig) {
    log_message("Server", "SHUTDOWN", "Server terminated by signal");
    exit(0);
}

void hex_to_bytes(const char *hex, unsigned char *bytes, size_t hex_len) {
    for (size_t i = 0; i < hex_len; i += 2) {
        sscanf(hex + i, "%2hhx", &bytes[i/2]);
    }
}

void reverse_string(char *str) {
    int length = strlen(str);
    for (int i = 0; i < length / 2; i++) {
        char temp = str[i];
        str[i] = str[length - i - 1];
        str[length - i - 1] = temp;
    }
}

int decrypt_text(const char *input_text, unsigned char **output_data, size_t *output_len) {   
// Make a copy of input to reverse
    char *reversed = strdup(input_text);
    if (!reversed) return -1;

    // Reverse the text
    reverse_string(reversed);

    // Remove newlines if any
    char *ptr = reversed;
    while (*ptr) {
        if (*ptr == '\n' || *ptr == '\r') *ptr = '\0';
        ptr++;
    }

    // Check if hex string is valid
    size_t hex_len = strlen(reversed);
    if (hex_len % 2 != 0) {
        free(reversed);
        return -1;
    }

    // Allocate memory for binary data
    *output_len = hex_len / 2;
    *output_data = malloc(*output_len);
    if (!*output_data) {
        free(reversed);
        return -1;
    }

    // Convert hex to bytes
    hex_to_bytes(reversed, *output_data, hex_len);

    free(reversed);
    return 0;
}

int save_to_database(const unsigned char *data, size_t data_len, char *filename) {
    // Create database directory if it doesn't exist
    mkdir(DATABASE_DIR, 0777);

    // Generate timestamp filename
    time_t now;
    time(&now);
    snprintf(filename, 64, "%ld.jpeg", now);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", DATABASE_DIR, filename);

    // Save the data to file
    FILE *file = fopen(filepath, "wb");
    if (!file) {
        return -1;
    }

    fwrite(data, 1, data_len, file);
    fclose(file);

    return 0;
}

int send_file(int client_socket, const char *filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", DATABASE_DIR, filename);

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        return -1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send file size first
    if (send(client_socket, &file_size, sizeof(file_size), 0) < 0) {
        fclose(file);
        return -1;
    }

    // Send file data
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            fclose(file);
            return -1;
        }
    }
    fclose(file);
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';

        if (strncmp(buffer, "DECRYPT:", 8) == 0) {
            // Handle decrypt request
            char *text_data = buffer + 8;

            unsigned char *decrypted_data;
            size_t decrypted_len;
            char filename[64];

            if (decrypt_text(text_data, &decrypted_data, &decrypted_len) == 0 &&
                save_to_database(decrypted_data, decrypted_len, filename) == 0) {

                log_message("Client", "DECRYPT", "Text data");
                log_message("Server", "SAVE", filename);

                send(client_socket, filename, strlen(filename), 0);
            } else {
                send(client_socket, "ERROR: Failed to decrypt or save file", 36, 0);
                log_message("Server", "ERROR", "Failed to decrypt or save file");
            }

            free(decrypted_data);
        }
        else if (strncmp(buffer, "DOWNLOAD:", 9) == 0) {
            // Handle download request
            char *filename = buffer + 9;

            log_message("Client", "DOWNLOAD", filename);

            if (send_file(client_socket, filename) == 0) {
                log_message("Server", "UPLOAD", filename);
            } else {
                send(client_socket, "ERROR: File not found", 20, 0);
                log_message("Server", "ERROR", "File not found for download");            
}
        }
        else if (strncmp(buffer, "EXIT", 4) == 0) {
            log_message("Client", "EXIT", "Client requested to exit");
            break;
        }
    }

    close(client_socket);
}

int main() {
    // Set up signal handler
    signal(SIGINT, handle_sigint);

    // Create server and database directories
    mkdir("server", 0777);
    mkdir(DATABASE_DIR, 0777);

    // Initialize log file
    log_message("Server", "STARTUP", "Server started");

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Bind failed");
    exit(EXIT_FAILURE);
}
 
   // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Daemonize the server
    if (fork() != 0) {
        exit(0);
    }

    setsid();
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Main server loop
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            log_message("Server", "ERROR", "Client connection failed");
            continue;
        }

        // Log client connection
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        char conn_info[64];
        snprintf(conn_info, sizeof(conn_info), "Client connected from %s", client_ip);
        log_message("Server", "CONNECT", conn_info);

        // Handle client in a child process
        if (fork() == 0) {
            close(server_socket);
            handle_client(client_socket);
            exit(0);
        }

        close(client_socket);
    }

    close(server_socket);
    return 0;
}