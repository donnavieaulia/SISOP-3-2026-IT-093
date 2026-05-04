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

broadcast pesan














