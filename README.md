# Laporan Resmi Praktikum SISOP Modul 3
## Anggota Kelompok

| No | Nama                     | NRP         |
|----|--------------------------|-------------|
| 1  | Tiara Putri Prasetya     | 5027241013  |
| 2  | Danuja Prasasta Bastu    | 5027241037  |
| 3  | Imam Mahmud Dalil Fauzan | 5027241100  |

# SOAL 1
1. Library yang diperlukan :
     - Membuat direktori ```(sys/stat.h)```
     - Input/output standar ```(stdio.h)```
     - Manajemen memori ```(stdlib.h)```
     - Operasi string ```(string.h)```
     - System calls ```(unistd.h)```
     - Socket programming ```(sys/socket.h, netinet/in.h, arpa/inet.h)```
     - Waktu ```(time.h)```
     - Directory operations ```(dirent.h)```
     - Error handling ```(errno.h)```

2. Konstanta:
   - `PORT`: Port untuk koneksi ke server (8080)
   - `BUFFER_SIZE`: Ukuran buffer untuk transfer data (1024 bytes)
   - `SECRETS_DIR`: Lokasi file teks rahasia ("client/secrets")
   - `OUTPUT_DIR`: Lokasi penyimpanan file hasil download ("client")

3. Fungsi Utama:
   - `display_menu()`: Menampilkan menu interaktif ke user
   - `list_text_files()`: Menampilkan daftar file teks yang tersedia di folder secrets
   - `connect_to_server()`: Membuat koneksi socket ke server
   - `send_decrypt_request()`: Mengirim permintaan dekripsi file ke server
   - `send_download_request()`: Mengirim permintaan download file ke server
   - `main()`: Fungsi utama yang menjalankan loop menu dan memproses pilihan user

4. Alur Kerja Client:
   - Menampilkan menu utama dengan 4 opsi
   - Untuk setiap opsi:
     1. List files: Menampilkan file teks yang ada di folder secrets
     2. Decrypt: Mengirim isi file teks ke server untuk didekripsi
     3. Download: Meminta file hasil dekripsi dari server
     4. Exit: Keluar dari program

5. Protokol Komunikasi dengan Server:
     - DECRYPT:<isi_file_teks>: Untuk permintaan dekripsi
     - DOWNLOAD:<nama_file>: Untuk permintaan download
     - EXIT: Untuk memberitahu server client akan keluar
     - Untuk dekripsi: Mengembalikan nama file hasil
     - Untuk download: Mengirim ukuran file terlebih dahulu, lalu isi file

6. Fitur Penting:
   - Validasi input user
   - Penanganan error yang baik
   - Pembuatan direktori otomatis jika belum ada
   - Tampilan yang user-friendly
   - Komunikasi yang jelas dengan server

7. Struktur Direktori Client:
   - File teks asli disimpan di client/secrets/
   - File hasil download disimpan di client/

8. Proses Transfer File:
   - Untuk download, client menerima ukuran file terlebih dahulu
   - Kemudian menerima data file per chunk (menggunakan buffer)
   - Menyimpan file secara streaming untuk efisiensi memori

Kode ini merupakan implementasi client yang berkomunikasi dengan server menggunakan socket TCP/IP, dengan antarmuka berbasis menu yang memungkinkan user untuk:
- Melihat daftar file teks yang tersedia
- Mengirim file teks untuk didekripsi oleh server
- Mendownload file hasil dekripsi dari server
- Keluar dari program dengan clean

kode ini dirancang untuk bekerja bersama dengan server yang telah dijelaskan sebelumnya, membentuk sistem RPC client-server yang lengkap.

# SOAL 2
Soal ini ada dua program utama:
`delivery_agent.c` – Menjalankan otomatisasi pengiriman Express menggunakan 3 agen.
`dispatcher.c` – Mengelola pengiriman Reguler, monitoring status, serta menampilkan daftar pesanan.

1. `delivery_agent.c`
Program ini bertanggung jawab atas pengiriman otomatis untuk pesanan Express.
Terdapat 3 agen pengiriman yang akan aktif secara paralel (menggunakan thread).
Setiap agen akan mengambil pesanan Express dari file CSV `delivery_order.csv`.
Setelah selesai mengantar, informasi akan dicatat ke dalam file log `delivery.log`.

2. `dispatcher.c`
Program ini berfungsi sebagai pengendali utama pengiriman Reguler.
Admin dapat memilih dan memproses pesanan satu per satu secara manual.
Tersedia fitur untuk melihat status pengiriman dan daftar pesanan.
Semua log aktivitas juga akan tercatat di `delivery.log`.

# SOAL 3
NO.3 THE LOST DUNGEON

Game the lost dungeon ini menggunakan client-server. Pemain menjelajahi dungeon, yaitu menampilkan main menu player, mengecek status player, melihat dan membeli senjata, melihat penyimpanan senjata, dan bertarung melawan musuh dan error handling jika opsi yang dipilih tidak ada. 
=============================== 
1. Sistem Client-Server: 
-`dungeon.c` berfungsi sebagai server yang menangani semua logika game
-`player.c` berfungsi sebagai client yang menampilkan antarmuka ke pemain

2. Progres Game:
-Pemain mulai dengan senjata dasar ("Fists") dan sejumlah gold
-Pemain bisa membeli senjata lebih kuat di toko
-Pemain bisa bertarung melawan musuh untuk mendapatkan lebih banyak gold

3. Sistem Pertarungan:
-Musuh muncul dengan HP acak
-Pemain menyerang dengan damage berdasarkan senjata yang dipakai 
===============================
WEAPON SHOP

Toko menawarkan 5 senjata dengan karakteristik berbeda dan passive yang berbeda:
1. Wooden Stick (Damage: 10, Harga: 50)
2. Flame Dagger (Damage: 15, Harga: 100)
3. Ice Mace (Damage: 20, Harga: 150, Passive: 10% Insta-Kill Chance)
4. Venom Sword (Damage: 25, Harga: 200, Passive: +30% Crit Chance)
5. Dragon Slayer (Damage: 40, Harga: 400) 
===============================
PENJELASAN PER FILE

1. `dungeon.c` (Server)
File ini berfungsi sebagai inti dari game atau game server, menangani semua logika permainan dan berkomunikasi dengan client.

Fungsi Utama:
-`Weapon weapons[]` = Struct yang menyimpan info senjata (nama, damage, harga, passive ability).
-`main()`: Membuat socket, bind ke port 8080, listen koneksi, menerima koneksi client via `accept()`, membaca perintah client,mengeksekusi fungsi sesuai perintah, dan mengirim respon e client
-`show_weapon_shop()`: Menampilkan daftar senjata yang tersedia di toko dan mengirim ke player
-`buy_weapon()`: Menangani proses pembelian senjata
-`battle_mode()`: Mengelola mode pertarungan melawan musuh

Fitur yang Ditangani:
-Menyimpan status pemain (gold, senjata, damage, jumlah kill)
-Mengelola inventory pemain
-Menghitung damage dalam pertarungan
-Memberikan reward setelah mengalahkan musuh
-Menangani semua perintah dari client (SHOW_STATS, SHOP, dll)

2. `player.c` (Client)
File ini berfungsi sebagai antarmuka pemain, menampilkan menu dan menangani output/input dari pemain.

Fungsi Utama:
-`main()`: Membuat koneksi ke server dan menampilkan menu utama
-Menampilkan pilihan menu dan mengirim perintah ke server
-Menerima dan menampilkan respons dari server

Fitur yang Ditangani:
-Menampilkan menu utama (Status, Toko, Inventory, Battle, Exit)
-Mengirim perintah ke server berdasarkan pilihan pemain
-Menerima dan menampilkan informasi dari server
-Menangani input pemain selama mode pertarungan

3. `shop.h` (Header File)
File ini berisi mendefinisikan struktur data dan deklarasi fungsi yang terkait dengan toko senjata.

Fungsi Utama:
-`struct Weapon`: Menyimpan properti senjata: name, damage, price, passive. 
-`show_weapon_shop()`: Untuk menampilkan toko daftar senjata. 
-`buy_weapon()`: Untuk logika pembelian senjata 
-`MAX_INVENTORY`: Untuk Batas maksimal inventory (10) 

# SOAL 4
1. `system.c`
Program admin yang bertugas:
- Mengelola data hunter dan dungeon
- Generate dungeon secara acak
- Melihat statistik hunter
- Fitur ban/reset hunter

Spesifikasi Dungeon:
Level Minimal : 1-5
ATK Reward   : 100-150
HP Reward    : 50-100  
DEF Reward   : 25-50
EXP Reward   : 150-300

2. `hunter.c`
Program player yang bisa:
- Registrasi/login hunter
- Melihat daftar dungeon
- Raid dungeon untuk mendapatkan reward
- Battle dengan hunter lain
- Sistem notifikasi
- Stat Awal Hunter:
Level : 1
ATK   : 10
HP    : 100
DEF   : 5
EXP   : 0

Cara me Run:
- Jalankan `system.c` terlebih dahulu (sebagai server):
```
gcc system.c -o system
./system
```

- Jalankan `hunter.c` (sebagai client):
```
gcc hunter.c -o hunter -lpthread
./hunter
```

Fitur Utama
Menu System (Admin)
```
1. Hunter Info   // Lihat stat semua hunter
2. Dungeon Info     // Lihat info dungeon
3. Generate Dungeon // Buat dungeon baru
4. Ban Hunter       // Blokir hunter
5. Reset Hunter     // Reset stat hunter
6. Exit
```

Menu Hunter (Player)
```
1. List Dungeon    // Lihat dungeon tersedia
2. Raid Dungeon       // Serang dungeon untuk dapat reward
3. Battle Hunter      // Duel dengan hunter lain
4. Notification       // Toggle notifikasi
5. Exit
```

Mekanisme Penting
Level Up:
Saat EXP ≥ 500, level naik dan EXP reset ke 0

Battle System:
Total power = ATK + HP + DEF
Yang kalah akan dihapus dari sistem
Pemenang dapat semua stat lawan

Shared Memory:
Menggunakan struktur SystemData yang dibagi antara system.c dan hunter.c
Data tersimpan selama system.c berjalan
