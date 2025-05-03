#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include "shm_common.h"

int shmid;
struct SystemData *system_data;

void generate_dungeon() {
    if (system_data->num_dungeons >= MAX_DUNGEONS) {
        printf("Max dungeons reached!\n");
        return;
    }

    struct Dungeon *d = &system_data->dungeons[system_data->num_dungeons];
    snprintf(d->name, 50, "Dungeon-%d", rand() % 1000);
    d->min_level = 1;
    d->exp = rand() % 151 + 150;
    d->atk = rand() % 51 + 100;
    d->hp = rand() % 51 + 50; 
    d->def = rand() % 26 + 25;
    d->shm_key = rand();

    system_data->num_dungeons++;
    printf("\nDungeon Generated!\nName: %s\nMin Level: %d\n", d->name, d->min_level);
}

void init_system() {
    key_t key = get_system_key();
    shmid = shmget(key, sizeof(struct SystemData), IPC_CREAT | 0666);
    
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    system_data = (struct SystemData *)shmat(shmid, NULL, 0);
    
    // Inisialisasi data jika baru pertama run
    if (system_data->num_hunters == -1) { // Gunakan flag inisialisasi
        memset(system_data, 0, sizeof(struct SystemData));
        system_data->num_hunters = 0;
        system_data->num_dungeons = 0;
    }
}

void show_dungeon_info() {
    printf("\n=== DUNGEON INFO ===\n");
    for (int i = 0; i < system_data->num_dungeons; i++) {
        struct Dungeon *d = &system_data->dungeons[i];
        printf("\n[Dungeon %d]\n", i + 1);
        printf("Name: %s\n", d->name);
        printf("Minimum Level: %d\n", d->min_level);
        printf("EXP Reward: %d\n", d->exp);
        printf("ATK: %d\n", d->atk);
        printf("HP: %d\n", d->hp);
        printf("DEF: %d\n", d->def);
        printf("Key: %d\n\n", d->shm_key);
    }
}

void show_hunter_info() {
    printf("\n=== HUNTER INFO ===\n");
    
    if (system_data->num_hunters == 0) {
        printf("Belum ada hunter terdaftar!\n");
        return;
    }

    for (int i = 0; i < system_data->num_hunters; i++) {
        printf("\n[%d] %s\n", i+1, system_data->hunters[i].username);
        printf("Level: %d \nEXP: %d\n", system_data->hunters[i].level, system_data->hunters[i].exp);
        printf("ATK: %d \nHP: %d \nDEF: %d\n", 
              system_data->hunters[i].atk,
              system_data->hunters[i].hp,
              system_data->hunters[i].def);
        printf("Status: %s\n", system_data->hunters[i].banned ? "BANNED" : "AKTIF");
    }
}

void ban_hunter() {
    char target[50];
    printf("Masukkan angka hunter yg mau di Ban: ");
    scanf("%s", target);
    
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, target) == 0) {
            system_data->hunters[i].banned = 1;
            printf("Hunter %s banned.\n", target);
            return;
        }
    }
    printf("Hunter tdk Ditemukan.\n");
}

void reset_hunter() {
    char target[50];
    printf("Masukkan angka Hunter yg mau di Reset: ");
    scanf("%s", target);
    
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, target) == 0) {
            system_data->hunters[i].level = 1;
            system_data->hunters[i].exp = 0;
            system_data->hunters[i].atk = 10;
            system_data->hunters[i].hp = 100;
            system_data->hunters[i].def = 5;
            system_data->hunters[i].banned = 0;
            printf("Hunter %s udh di Reset.\n", target);
            return;
        }
    }
    printf("Hunter tdk Ditemukan.\n");
}

void clean_up(int signum) {
    shmdt(system_data);
    shmctl(shmid, IPC_RMID, NULL);
    printf("\nShared memory cleaned. Exiting.\n");
    exit(0);
}

int main() {
    key_t system_key = get_system_key();
    shmid = shmget(system_key, sizeof(struct SystemData), IPC_CREAT | 0666);
    system_data = (struct SystemData *)shmat(shmid, NULL, 0);
    
    // Inisialisasi
    if (system_data->initialized != 1) {
        memset(system_data, 0, sizeof(struct SystemData));
        system_data->initialized = 1;
        system_data->num_hunters = 0;
        system_data->num_dungeons = 0;
    }

    signal(SIGINT, clean_up);
    init_system();

    while (1) {
        printf("\n=== SYSTEM MENU ===\n");
        printf("1. Hunter Info\n");
        printf("2. Dungeon Info\n");
        printf("3. Generate Dungeon\n");
        printf("4. Ban Hunter\n");
        printf("5. Reset Hunter\n");
        printf("6. Exit\n");
        printf("Choice: ");

        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1: show_hunter_info();
                break;
            case 2: show_dungeon_info();
                break;
            case 3: generate_dungeon();
                break;
            case 4: ban_hunter();
                break;
            case 5: reset_hunter();
                break;
            case 6: clean_up(0);
                break;

            default: printf("Pilih 1-6 saja.\n");
        }
    }
    return 0;
}