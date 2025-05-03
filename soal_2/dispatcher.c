#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define MAX_ORDER 100
#define MAX_STRING 100

typedef struct {
    char name[MAX_STRING];
    char address[MAX_STRING];
    char type[MAX_STRING]; // Express / Reguler
    char status[MAX_STRING]; // Pending / Delivered by Agent <X>
} Order;

Order *orders;

void write_log(const char *agent, const char *name, const char *address, const char *type) {
    FILE *log = fopen("delivery.log", "a");
    if (!log) return;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_buf[100];
    strftime(time_buf, sizeof(time_buf), "%d/%m/%Y %H:%M:%S", tm_info);

    fprintf(log, "[%s] [AGENT %s] %s package delivered to %s in %s\n",
            time_buf, agent, type, name, address);

    fclose(log);
}

void deliver_reguler(const char *agent_name, const char *recipient_name) {
    for (int i = 0; i < MAX_ORDER; i++) {
        if (strcmp(orders[i].type, "Reguler") == 0 && //cari kondisi pesanan yg reguler & sdg. pending dan nama penerimanya sesuai dg. recipient
            strcmp(orders[i].status, "Pending") == 0 &&
            strcmp(orders[i].name, recipient_name) == 0) {

            //ubah statusnya sdg. diproses
            snprintf(orders[i].status, MAX_STRING, "Delivered by Agent %s", agent_name);

            write_log(agent_name, orders[i].name, orders[i].address, "Reguler");

            printf("Pesanan %s berhasil dikirim oleh Agent %s!\n", recipient_name, agent_name);
            return;
        }
    }
    printf("Pesanan untuk %s tidak ditemukan atau sudah dikirim.\n", recipient_name);
}

void check_status(const char *name) { //u. cari status pesanan bds. nama pnrima
    for (int i=0 ; i<MAX_ORDER ; i++) {
        if (strcmp(orders[i].name, name) == 0) {
            printf("Status for %s: %s\n", name, orders[i].status);
            return;
        }
    }
    printf("Pesanan untuk %s tidak ditemukan.\n", name);
}

void list_orders() { //tampilkan smua dftar pesanan
    printf("Daftar Semua Pesanan:\n");
    for (int i=0 ; i<MAX_ORDER ; i++) {
        if (strlen(orders[i].name) > 0) {
            printf("Nama: %s | Status: %s\n", orders[i].name, orders[i].status);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("./dispatcher -deliver <pengirim> <penerima>\n");
        printf("./dispatcher -status <penerima>\n");
        printf("./dispatcher -list\n");
        exit(1);
    }

    key_t key = 1234; //key yg di set di delivery_agent.c
    int shmid;

    shmid = shmget(key, sizeof(Order) * MAX_ORDER, 0666); //buat/ambil sh.memory 
    if (shmid < 0) {
        perror("shmget Gagal Dibuat !");
        exit(1);
    }

    orders = (Order *) shmat(shmid, NULL, 0); //nempelin sh.mem ke alamat mem lokal
    if ((void *) orders == (void *) -1) {
        perror("shmat Error !");
        exit(1);
    }

    if (strcmp(argv[1], "-deliver") == 0 && argc == 4) {
        deliver_reguler(argv[2], argv[3]);

    } else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        check_status(argv[2]);

    } else if (strcmp(argv[1], "-list") == 0) {
        list_orders();

    } else {
        printf("Perintah tidak valid.\n");
        printf("Sintaks:\n");
        printf("./dispatcher -deliver <pengirim> <penerima>\n");
        printf("./dispatcher -status <RecipientName>\n");
        printf("./dispatcher -list\n");
    }

    shmdt(orders); //lepaskan sh.mem yg udh di tempelin (shmat)
    return 0;
}
