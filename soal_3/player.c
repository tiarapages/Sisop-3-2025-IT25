#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket gagal\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Koneksi gagal\n");
        return -1;
    }

    while (1) {
        printf("\n------ MAIN MENU ------\n");
        printf("1. Show Player Stats\n");
        printf("2. Shop (Buy Weapons)\n");
        printf("3. View Inventory & Equip Weapons\n");
        printf("4. Battle Mode\n");
        printf("5. Exit Game\n");
        printf("Choose an option: ");

        int choice;
        scanf("%d", &choice);
        getchar(); 

        char command[64];
        if (choice == 1) {
            strcpy(command, "SHOW_STATS");
        } else if (choice == 2) {
            strcpy(command, "SHOP");
        } else if (choice == 3) {
            strcpy(command, "VIEW_INVENTORY");
        } else if (choice == 4) {
            strcpy(command, "BATTLE_MODE");
        } else if (choice == 5) {
            strcpy(command, "EXIT");
            send(sock, command, strlen(command), 0);
            printf("Keluar dari game.\n");
            break;
        } else {
            printf("Pilihan tidak valid.\n");
            continue;
        }

        send(sock, command, strlen(command), 0);

        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, sizeof(buffer));
    
        if (strcmp(command, "SHOP") == 0) {
            printf("%s", buffer);  
            
            int weapon_choice;
            scanf("%d", &weapon_choice);
            getchar();

            char weapon_number[16];
            snprintf(weapon_number, sizeof(weapon_number), "%d", weapon_choice);
            send(sock, weapon_number, strlen(weapon_number), 0);
            
            memset(buffer, 0, sizeof(buffer));
            read(sock, buffer, sizeof(buffer));
            printf("%s\n", buffer);

        } else if (strcmp(command, "VIEW_INVENTORY") == 0) {
            printf("%s", buffer);  
            
            int weapon_choice;
            scanf("%d", &weapon_choice);
            getchar();

            char weapon_number[10];
            snprintf(weapon_number, sizeof(weapon_number), "%d", weapon_choice);
            send(sock, weapon_number, strlen(weapon_number), 0);
            
            memset(buffer, 0, sizeof(buffer));
            read(sock, buffer, sizeof(buffer));
            printf("%s\n", buffer);

        } else if (strcmp(command, "BATTLE_MODE") == 0) {
            send(sock, "", 1, 0);
            while (1) {
                memset(buffer, 0, sizeof(buffer));
                read(sock, buffer, sizeof(buffer));

                if (strstr(buffer, "Kembali ke menu utama")) {
                    printf("%s", buffer);
                    break;
                }

                printf("%s", buffer);

                if (strstr(buffer, "Pilih aksi")) {
                    int battle_choice;
                    scanf("%d", &battle_choice);
                    getchar();
                    char battle_cmd[16];
                    snprintf(battle_cmd, sizeof(battle_cmd), "%d", battle_choice);
                    send(sock, battle_cmd, strlen(battle_cmd), 0);
                }
            }
        } else {
            printf("%s\n", buffer);
        }
    }
    
    close(sock);
    return 0;
}