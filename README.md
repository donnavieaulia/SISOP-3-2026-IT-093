# SISOP-3-2026-IT-093 #

## NAMA : Donnavie Aulia NRP: 5027251093 ##

Soal 1 - Present Day, Present Time

Pada soal ini, dibuat sebuah sistem komunikasi client-server (The Wired) menggunakan socket TCP. Server berfungsi sebagai pusat komunikasi yang mampu menangani banyak client secara bersamaan tanpa menggunakan `fork()`, melainkan dengan `select()`.
Setiap client harus melakukan autentikasi menggunakan username unik dan password sebelum dapat bergabung ke sistem. Setelah terhubung, client dapat mengirim pesan yang akan di-broadcast ke seluruh client lain yang aktif.
Selain itu, server juga harus mencatat seluruh aktivitas (connect, disconnect, dan chat) ke dalam file log `(history.log)` dengan format timestamp.

Langkah 1: Menyiapkan dasar komunikasi jaringan. Server perlu membuat socket sebagai titik komunikasi menggunakan fungsi `socket()`, kemudian mengaitkannya ke port tertentu dengan `bind()`. Proses ini memungkinkan server siap menerima koneksi dari client.

```c
sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

```

 Langkah 2: Membuat protokol sebagai format komunikasi agar data antara Client dan Server tidak salah dipahami.
 Setiap pesan dibungkus dalam `struct` yang berisi pengirim, perintah, dan isi pesan,sehingga pertukaran data lebih terstruktur.

 ```c
typedef struct {
    int type;                 // jenis pesan
    char name[MAX_NAME];      // username
    char message[MAX_MSG];    // isi pesan
} Message;

```

Langkah 3: Pada tahap ini dibuat fungsi helper pada file `protocol.c` untuk mempermudah pembuatan pesan.
Namun pada implementasi program, fungsi ini belum digunakan secara penuh karena komunikasi masih dilakukan secara manual menggunakan struct.


```c
Message msg;
msg.type = MSG_NAME;
strcpy(msg.message, name);
send(sock, &msg, sizeof(msg), 0);

```
langkah 4: diimplementasikan pada file wired.c, yang berperan sebagai server utama dalam sistem.
Pada bagian ini dilakukan pembuatan socket, binding, listening, serta pengelolaan banyak client 
menggunakan `select()`. Selain itu, server juga menangani autentikasi user, validasi username, broadcast pesan, dan manajemen koneksi client.

 Pembuatan socket, binding, listening

```c
server_fd = socket(AF_INET, SOCK_STREAM, 0);

address.sin_family = AF_INET;
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(PORT);

bind(server_fd, (struct sockaddr *)&address, sizeof(address));
listen(server_fd, 5);
```

Pengelolaan banyak client menggunakan select()

```c
fd_set readfds;

FD_ZERO(&readfds);
FD_SET(server_fd, &readfds);

for (int i = 0; i < MAX_CLIENTS; i++) {
    int sd = client_sockets[i];
    if (sd > 0) FD_SET(sd, &readfds);
    if (sd > max_sd) max_sd = sd;
}

select(max_sd + 1, &readfds, NULL, NULL, NULL);

```
Menangani autentikasi user

```c
read(new_socket, password, sizeof(password));
password[strcspn(password, "\n")] = 0;

if (strcmp(password, "bebas") != 0) {
    char *msg = "Password salah!\n";
    send(new_socket, msg, strlen(msg), 0);
    close(new_socket);
    continue;
}
```

Validasi username

```c
for (int i = 0; i < MAX_CLIENTS; i++) {
    if (strcmp(client_names[i], name) == 0) {
        duplicate = 1;
        break;
    }
}
```

Broadcast pesan

```c
for (int j = 0; j < MAX_CLIENTS; j++) {
    if (client_sockets[j] != 0 && client_sockets[j] != sd) {
        char msg[1200];
        sprintf(msg, "[%s]: %s\n", client_names[i], buffer);
        send(client_sockets[j], msg, strlen(msg), 0);
    }
}
```
Managemen koneksi client

```c

cliend masuk

client_sockets[i] = new_socket;
strcpy(client_names[i], name);

client keluar

if (valread <= 0) {
    close(sd);
    client_sockets[i] = 0;
}
```

Langkah 5: Client berfungsi untuk terhubung ke server menggunakan `socket()` dan `connect()`. Setelah terkoneksi, user memasukkan username dan password, kemudian client mengirimkan data tersebut ke server. Selanjutnya, client dapat terus mengirim pesan (chat), dan koneksi dapat diakhiri dengan perintah `/exit`.


Membuat socket client dan menghubungkan ke server
```c
sock = socket(AF_INET, SOCK_STREAM, 0);

serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(PORT);

inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
```

Input username & password
```c
char name[100];
printf("Enter your name: ");
fgets(name, sizeof(name), stdin);

char password[100];
printf("Enter password: ");
fgets(password, sizeof(password), stdin);
```

Mengirim username ke user

```c
send(sock, name, strlen(name), 0);
```

Looping
```c
while (1) {
```

Menginput pesan

```c
fgets(message, sizeof(message), stdin);
```

Mengirim pesan ke server

```c
send(sock, message, strlen(message), 0);
```

Kirim struct message

```c
Message msg;
msg.type = MSG_NAME;
strcpy(msg.message, name);
send(sock, &msg, sizeof(msg), 0);

Message auth;
auth.type = MSG_AUTH;
strcpy(auth.message, password);
send(sock, &auth, sizeof(auth), 0);
```

Langkah 6: Diimplementasikan pada file `wired.c` melalui fungsi `write_log()`. Fungsi ini digunakan untuk mencatat seluruh aktivitas sistem ke dalam file history.log, termasuk saat server berjalan, client terhubung, client mengirim pesan, dan client keluar. Logging dilakukan dengan format timestamp menggunakan fungsi `time()`, `localtime()`, dan `strftime()` agar setiap aktivitas dapat tercatat secara terstruktur.

```c
void write_log(const char *type, const char *msg) {
    FILE *f = fopen("history.log", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timebuf[50];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);

    fprintf(f, "[%s] [%s] %s\n", timebuf, type, msg);
    fclose(f);
}
```
Log saat server start

```c
write_log("System", "SERVER ONLINE");
```

Log client connect

```c
char logmsg[200];
sprintf(logmsg, "User '%s' connected", name);
write_log("System", logmsg);
```

Log pesan user

```c
char logmsg[200];
sprintf(logmsg, "[%s]: %s", client_names[i], buffer);
write_log("User", logmsg);
```

Log client disconnect

```c
char logmsg[200];
sprintf(logmsg, "User '%s' disconnected", client_names[i]);
write_log("System", logmsg);
```
Tahap 7: Pada tahap ini, server harus mampu menangani banyak client secara bersamaan tanpa menggunakan `fork()` atau `thread`. Untuk itu digunakan mekanisme I/O multiplexing dengan fungsi `select()`.

```c
void write_log(const char *type, const char *msg) {
    FILE *f = fopen("history.log", "a");   // buka file log
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%02d:%02d:%02d] [%s] %s\n",
            t->tm_hour, t->tm_min, t->tm_sec, type, msg);
    // tulis waktu + tipe + pesan

    fclose(f); // tutup file
}
```
Langkah 8: Server melakukan broadcast dengan mengirim pesan dari satu client ke seluruh client lain yang sedang aktif menggunakan fungsi `send()`.

```c
for (int j = 0; j < MAX_CLIENTS; j++) {
    if (client_sockets[j] != 0 && client_sockets[j] != sd) {
        send(client_sockets[j], msg, strlen(msg), 0);
        // kirim pesan ke semua client kecuali pengirim
    }
}
```
Langkah 9: Server mendeteksi client keluar melalui `read()`. Jika tidak ada data yang diterima, maka koneksi ditutup dan client dihapus dari daftar.

```c
int valread = read(sd, buffer, sizeof(buffer));

if (valread <= 0) {
    close(sd);                 // tutup koneksi client
    client_sockets[i] = 0;     // hapus dari daftar client
}
```

Untuk memastikan program berfungsi sesuai dengan yang diharapkan, dilakukan serangkaian pengujian melalui beberapa tahapan yang telah ditentukan

compile: 
```c
gcc wired.c -o wired
gcc navi.c -o navi
```
Server dijalankan terlebih dahulu dengan mengeksekusi file wired pada satu terminal untuk mengaktifkan layanan.
```c
/wired.
```

Selanjutnya, client dijalankan menggunakan navi pada terminal yang berbeda. Untuk simulasi lebih dari satu user, cukup buka terminal tambahan dan jalankan navi kembali.

```c
./navi
```

Periksa file history.log untuk memastikan bahwa seluruh aktivitas sistem telah tercatat dengan benar dan dilengkapi dengan timestamp yang sesuai.

### Hasil Output ###

Berikut adalah cuplikan log activity yang berhasil dicatat sistem:
<img width="1400" height="1358" alt="image" src="https://github.com/user-attachments/assets/69e62597-cfa7-4467-945b-b59bde1758ec" />




























