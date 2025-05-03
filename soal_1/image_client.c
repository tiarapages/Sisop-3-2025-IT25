#include <sys/stat.h>  // For mkdir()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SECRETS_DIR "client/secrets"
#define OUTPUT_DIR "client"

void display_menu() {
    printf("\n=== The Legend of Rootkids ===\n");
    printf("1. List available text files\n");
    printf("2. Decrypt and save a text file\n");
    printf("3. Download a JPEG from server\n");
    printf("4. Exit\n");
    printf("Choose an option: ");
}

void list_text_files() {
    DIR *dir;
    struct dirent *ent;

    printf("\nAvailable text files in %s:\n", SECRETS_DIR);

    if ((dir = opendir(SECRETS_DIR)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, ".txt")) {
                printf("- %s\n", ent->d_name);
            }
        }
        closedir(dir);
    } else {
        perror("Could not open secrets directory");
    }
}

int connect_to_server() {
 int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\nSocket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }

    return sock;
}

int send_decrypt_request(int sock, const char *filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/%s", SECRETS_DIR, filename);

    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return -1;
    }

    // Read file content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_content = malloc(file_size + 1);
    if (!file_content) {
        fclose(file);
        return -1;
    }

    fread(file_content, 1, file_size, file);
    file_content[file_size] = '\0';
    fclose(file);

    // Prepare and send request
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "DECRYPT:%s", file_content);
    free(file_content);

    if (send(sock, request, strlen(request), 0) < 0) {
        printf("Error sending decrypt request\n");
        return -1;
    }

    // Receive response
    char response[BUFFER_SIZE];
    int bytes_received = recv(sock, response, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Error receiving response from server\n");
        return -1;
    }

    response[bytes_received] = '\0';

    if (strncmp(response, "ERROR:", 6) == 0) {
        printf("Server error: %s\n", response + 6);
        return -1;
    }

    printf("File successfully decrypted and saved as: %s\n", response);
    return 0;
}

int send_download_request(int sock, const char *filename) {
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "DOWNLOAD:%s", filename);

    if (send(sock, request, strlen(request), 0) < 0) {
        printf("Error sending download request\n");
        return -1;
    }

    // First receive file size
 long file_size;
    if (recv(sock, &file_size, sizeof(file_size), 0) <= 0) {
        printf("Error receiving file size\n");
        return -1;
    }

    if (file_size <= 0) {
        printf("Error: Invalid file size received\n");
        return -1;
    }

    // Create output directory if it doesn't exist
   // Create output directory if it doesn't exist
if (mkdir(OUTPUT_DIR, 0777) == -1 && errno != EEXIST) {
    printf("Error creating output directory\n");
    return -1;
}

    // Prepare output file path
    char output_path[256];
    snprintf(output_path, sizeof(output_path), "%s/%s", OUTPUT_DIR, filename);

    FILE *output_file = fopen(output_path, "wb");
    if (!output_file) {
        printf("Error creating output file\n");
        return -1;
    }

    // Receive file data
    unsigned char buffer[BUFFER_SIZE];
    long total_received = 0;

    while (total_received < file_size) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Error receiving file data\n");
            fclose(output_file);
            return -1;
        }

        fwrite(buffer, 1, bytes_received, output_file);
        total_received += bytes_received;
    }

    fclose(output_file);
 printf("File successfully downloaded to: %s\n", output_path);
    return 0;
}

int main() {
    int option;
    int sock;
    char filename[256];

    while (1) {
        display_menu();
        scanf("%d", &option);
        getchar(); // Consume newline

        switch (option) {
            case 1:
                list_text_files();
                break;

            case 2:
                printf("Enter filename to decrypt (from secrets folder): ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0'; // Remove newline

                sock = connect_to_server();
                if (sock < 0) {
                    printf("Failed to connect to server\n");
                    break;
                }

                if (send_decrypt_request(sock, filename) == 0) {
                    printf("Decryption successful!\n");
                }

                close(sock);
                break;

            case 3:
                printf("Enter filename to download (e.g., 1234567890.jpeg): ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0'; // Remove newline

                sock = connect_to_server();
                if (sock < 0) {
                    printf("Failed to connect to server\n");
                   break;
                }

                if (send_download_request(sock, filename) == 0) {
                    printf("Download successful!\n");
                }

                close(sock);
                break;

            case 4:
                // Send exit request if connected
                sock = connect_to_server();
                if (sock >= 0) {
                    send(sock, "EXIT", 4, 0);
                    close(sock);
                }
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid option. Please try again.\n");
        }
    }

    return 0;
}