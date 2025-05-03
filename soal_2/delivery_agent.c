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
    char type[MAX_STRING]; //expres ato requler
    char status[MAX_STRING]; //pending ato delivered by x
} Order;

Order *orders;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //tipe data u. mutual exclusion = menghindari balapan

void write_log(const char *agent, const char *name, const char *address) { //fungsi buat file log    
    FILE *log = fopen("delivery.log", "a"); //
    if (!log) return;

    time_t t = time(NULL); //buat waktu (timestamp)
    struct tm *tm_info = localtime(&t);
    char time_buf[100];
    strftime(time_buf, sizeof(time_buf), "%d/%m/%Y %H:%M:%S", tm_info); //mengubah format waktu

    fprintf(log, "[%s] [%s] Express package delivered to %s in %s\n", time_buf, agent, name, address);
    fclose(log);
}

void *agent_worker(void *arg) { //fungsi u. mnglola kiriman paket
    char *agent = (char *)arg;

    while (1) { //trus-menerus berjalan smpai dimatikan manual
        pthread_mutex_lock(&lock); //me lock mutex = memastikan hanya 1 thread(agen) yg mengelola data orderan
        
        for (int i=0 ; i<MAX_ORDER ; i++) {
            if (strcmp(orders[i].type, "Express") == 0 && //ngecek 2 kodnisi : express & sdg. pending
                strcmp(orders[i].status, "Pending") == 0){

                //ubah statusnya sdg diproses
                snprintf(orders[i].status, MAX_STRING, "Delivered by %s", agent);

                write_log(agent, orders[i].name, orders[i].address);

                break; //cm ambil 1 pesanan per agen
            }
        }
        pthread_mutex_unlock(&lock); //lepas mutex

        sleep(1); //kasih jeda biar thread gk meriksa terus = 100% cpu
    }

    return NULL;
}

int main() {
    key_t key = 1234; //set key nya 1234
    int shmid;
    
    shmid = shmget(key, sizeof(Order) * MAX_ORDER, IPC_CREAT | 0666); //shmget = mbuat share mem , akses 0666 = r+w U,G,O
    if (shmid < 0) {
        perror("shmget Gagal Dibuat !");
        exit(1);
    }

    orders = (Order *) shmat(shmid, NULL, 0); //shmat=u.nempelin/ngambil sh.memory yg udh dibuat td
    if ((void *) orders == (void *) -1) {
        perror("shmat Error !");
        exit(1);
    }

    FILE *file = fopen("delivery_order.csv", "r");
    if (file == NULL) {
        perror("Error File CSV Gagal Dibuka !");
        exit(1);
    }

    char line[256]; //buffer (tampungan) 1 baris dr file csv
    int idx = 0; 

    while (fgets(line, sizeof(line), file)) { //ngambil 1 baris di csv tp dibatasi 256 kar.
        if (idx >= MAX_ORDER) {
            printf("Order melebihi kapasitas maksimal.\n");
            break;
        }

        line[strcspn(line, "\n")] = 0; //hapus newline diakhir nya

        //parsing(mengolah) csv, memecah jd 3 bagian : Nama,Alamat,Tipe 
        char *token = strtok(line, ",");
        if (token != NULL) strncpy(orders[idx].name, token, MAX_STRING);

        token = strtok(NULL, ",");
        if (token != NULL) strncpy(orders[idx].address, token, MAX_STRING);

        token = strtok(NULL, ",");
        if (token != NULL) strncpy(orders[idx].type, token, MAX_STRING);

        strncpy(orders[idx].status, "Pending", MAX_STRING); //buat status 'pending' dr tiap pesanan yg dibaca
        idx++;
    }

    fclose(file);
    printf("Sukses membaca dan menyimpan %d order ke shared memory!\n", idx);

    pthread_t agents[3]; //deklarasi dg. tipedata pthread_t = nyimpen data thread
    char *names[3] = {"AGENT A", "AGENT B", "AGENT C"};

    for (int i=0 ; i<3 ; i++) {
        pthread_create(&agents[i], NULL, agent_worker, names[i]); //buat 3 thread agen nya
    }

    for (int i=0 ; i<3 ; i++) {
        pthread_join(agents[i], NULL); //tunggu ke3 agen nyelesaiin proses pesanan mrka
    }

    shmdt(orders); //lepas sh.mem orders 
    return 0;
}
