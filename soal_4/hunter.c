#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <pthread.h>
#include "shm_common.h"

struct SystemData *system_data;
int shmid;
pthread_mutex_t notif_mutex = PTHREAD_MUTEX_INITIALIZER;
bool running = true;
struct Hunter *current_user = NULL;

void attach_shm() {
    key_t system_key = get_system_key();
    shmid = shmget(system_key, sizeof(struct SystemData), 0666);
    if (shmid == -1) {
        printf("Jalanin system.c dulu !\n");
        exit(1);
    }

    system_data = (struct SystemData *)shmat(shmid, NULL, 0);
    if (system_data == (void *)-1) {
        perror("Gagal mengakses shared memory");
        exit(1);
    }
}

void *notification_thread(void *arg) {
    while (running) {
        pthread_mutex_lock(&notif_mutex);
        if (current_user && current_user->notif) {
            printf("\n=== NOTIFIKASI ===\n");
            for (int i = 0; i < system_data->num_dungeons; i++) {
                if (current_user->level >= system_data->dungeons[i].min_level) {
                    printf("- %s (Level %d+)\n", 
                          system_data->dungeons[i].name,
                          system_data->dungeons[i].min_level);
                }
            }
            sleep(5);  // Notifikasi hanya muncul setiap 5 detik
        }
        pthread_mutex_unlock(&notif_mutex);
    }
    return NULL;
}

void debug_shared_memory() {
    printf("\n=== DEBUG INFO ===\n");
    printf("Total Hunters: %d\n", system_data->num_hunters);
    printf("Total Dungeons: %d\n", system_data->num_dungeons);
    
    printf("\nCurrent User:\n");
    if(current_user) {
        printf("Name: %s\n", current_user->username);
        printf("Level: %d\n", current_user->level);
    } else {
        printf("No current user\n");
    }
    
    printf("\nDungeons:\n");
    for(int i = 0; i < system_data->num_dungeons; i++) {
        printf("%d. %s (Lvl %d)\n", i+1, 
              system_data->dungeons[i].name, 
              system_data->dungeons[i].min_level);
    }
}

void register_hunter() {
    if (system_data->num_hunters >= MAX_HUNTERS) {
        printf("Jumlah hunter sudah penuh!\n");
        return;
    }

    char uname[50];
    printf("Username: ");
    scanf("%s", uname);

    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, uname) == 0) {
            printf("Username sudah terdaftar!\n");
            return;
        }
    }

    struct Hunter new_hunter = {
        .level = 1,
        .exp = 0,
        .atk = 10,
        .hp = 100,
        .def = 5,
        .banned = 0,
        .notif = 0
    };
    strcpy(new_hunter.username, uname);

    system_data->hunters[system_data->num_hunters] = new_hunter;
    system_data->num_hunters++;
    printf("Registrasi berhasil!\n");
}

void hunter_menu();

void login() {
    char uname[50];
    printf("Username: ");
    scanf("%s", uname);
    
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, uname) == 0) {
            current_user = &system_data->hunters[i];
            current_user->notif = false;
            printf("Login berhasil!\n");
            running = true;
            return hunter_menu();
        }
    }
    printf("User tidak ditemukan!\n");
}

void list_dungeons() {
    printf("\n=== DUNGEON TERSEDIA ===\n");
    
    if (system_data->num_dungeons == 0) {
        printf("Belum ada dungeon tersedia!\n");
        return;
    }

    // DEBUG: Tampilkan semua dungeon terlepas dari level
    printf("[DEBUG] Daftar semua dungeon:\n");
    for (int i = 0; i < system_data->num_dungeons; i++) {
        printf("%d. %s (Level %d+) %s\n", 
              i+1, 
              system_data->dungeons[i].name,
              system_data->dungeons[i].min_level,
              (current_user->level >= system_data->dungeons[i].min_level) ? 
              "[BISA DIAKSES]" : "[LEVEL TERLALU RENDAH]");
    }

    // Hanya tampilkan yang bisa diakses
    printf("\nDungeon yang tersedia untukmu:\n");
    int available = 0;
    for (int i = 0; i < system_data->num_dungeons; i++) {
        if (current_user->level >= system_data->dungeons[i].min_level) {
            printf("%d. %s (Level %d+)\n", 
                  available+1, 
                  system_data->dungeons[i].name,
                  system_data->dungeons[i].min_level);
            available++;
        }
    }
    
    if (available == 0) {
        printf("Tidak ada dungeon yang bisa diakses!\n");
        printf("Level Anda: %d\n", current_user->level);
    }
}

void raid_dungeon() {
    list_dungeons();
    if (system_data->num_dungeons == 0) {
        printf("Tdk ada dungeon tersedia!\n");
        return;
    }

    printf("Pilih Dungeon: ");
    int choice;
    scanf("%d", &choice);
    
    if (choice < 1 || choice > system_data->num_dungeons) {
        printf("Pilihan tidak valid!\n");
        return;
    }

    struct Dungeon *d = &system_data->dungeons[choice-1];
    
    if (current_user->banned) {
        printf("Anda dibanned, tidak bisa raid!\n");
        return;
    }
    
    if (current_user->level < d->min_level) {
        printf("Level Anda belum cukup!\n");
        return;
    }

    printf("Raid Berhasil! Mendapatkan:\n");
    printf("ATK: %d\nHP: %d\nDEF: %d\nEXP: %d\n",
           d->atk, d->hp, d->def, d->exp);
           
    current_user->atk += d->atk;
    current_user->hp += d->hp;
    current_user->def += d->def;
    current_user->exp += d->exp;
    
    if (current_user->exp >= 500) {
        current_user->level++;
        current_user->exp = 0;
        printf("Level Up! Sekarang level %d\n", current_user->level);
    }

    for (int i = choice-1; i < system_data->num_dungeons-1; i++) {
        system_data->dungeons[i] = system_data->dungeons[i+1];
    }
    system_data->num_dungeons--;
}

void battle_hunter() {
    if (system_data->num_hunters < 2) {
        printf("\nTidak ada hunter lain untuk ditantang!\n");
        return;
    }

    printf("\n=== DAFTAR HUNTER ===\n");
    for (int i = 0; i < system_data->num_hunters; i++) {
        if (strcmp(system_data->hunters[i].username, current_user->username) != 0) {
            int power = system_data->hunters[i].atk + 
                        system_data->hunters[i].hp + 
                        system_data->hunters[i].def;
            printf("%d. %s (Power: %d)\n", i+1, 
                  system_data->hunters[i].username, power);
        }
    }

    printf("Pilih lawan (nomor): ");
    int choice;
    scanf("%d", &choice);
    choice--; // Konversi ke index array

    if (choice < 0 || choice >= system_data->num_hunters || 
        strcmp(system_data->hunters[choice].username, current_user->username) == 0) {
        printf("Pilihan tidak valid!\n");
        return;
    }

    struct Hunter *enemy = &system_data->hunters[choice];
    
    int my_power = current_user->atk + current_user->hp + current_user->def;
    int enemy_power = enemy->atk + enemy->hp + enemy->def;

    printf("\n=== BATTLE ===\n");
    printf("%s (Power: %d) vs %s (Power: %d)\n", 
          current_user->username, my_power, 
          enemy->username, enemy_power);

    if (my_power > enemy_power) {
        //yg menang dpt stat nya lawan
        current_user->atk += enemy->atk;
        current_user->hp += enemy->hp;
        current_user->def += enemy->def;

        //hapus akun lawan dr sistem wkwkwk
        for (int i = choice; i < system_data->num_hunters-1; i++) {
            system_data->hunters[i] = system_data->hunters[i+1];
        }
        system_data->num_hunters--;

        printf("\nAnda MENANG! Stat %s ditambahin ke Akunmu\n", enemy->username);
    } else {
        printf("\nAnda KALAH! Akunmu akan dihapus (WKWKWK jgn nangis)\n");
        exit(0); //lsg keluar dr program (akun yg kalah)
    }
}

void hunter_menu() {
    debug_shared_memory();
    pthread_t notif_thread;
    pthread_create(&notif_thread, NULL, notification_thread, NULL);

    while (1) {
        printf("\n=== MENU %s ===\n", current_user->username);
        printf("1. List Dungeon\n");
        printf("2. Raid Dungeon\n");
        printf("3. Battle Hunter\n");
        printf("4. Notifikasi\n");
        printf("5. Keluar\n");
        printf("Pilihan: ");

        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1: list_dungeons();
            break;
            case 2: raid_dungeon();
            break;
            case 3: battle_hunter();
            break;
            case 4: 
                current_user->notif = !current_user->notif;
                printf("Notifikasi %s\n", 
                      current_user->notif ? "AKTIF" : "NONAKTIF");
                break;
            case 5: 
                running = false;
                pthread_join(notif_thread, NULL);
                return;
            default: printf("Pilih 1-5 aja !\n");
        }
    }
}

void handle_interrupt(int sig) {
    printf("\nKeluar dari program...\n");
    running = false;
    exit(0);
}

int main() {
    signal(SIGINT, handle_interrupt);
    attach_shm();

    while (1) {
        printf("\n=== MENU UTAMA ===\n");
        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Keluar\n");
        printf("Pilihan: ");

        int choice;
        scanf("%d", &choice);

        switch (choice) {
            case 1: register_hunter(); break;
            case 2: login(); break;
            case 3: return 0;
            default: printf("Pili 1-3 aja !\n");
        }
    }
}