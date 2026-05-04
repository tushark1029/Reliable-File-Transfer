#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 4

typedef struct {
    int seq_no;
    int size;
    char data[BUFFER_SIZE];
} Packet;

typedef struct {
    int ack_no;
} Ack;

void encrypt_decrypt(char *data, int size) {
    for (int i = 0; i < size; i++)
        data[i] ^= 'K';
}

SSL_CTX *ctx;

void *handle_client(void *arg) {

    SSL *ssl = (SSL *)arg;
    int loss_done[1000] = {0};  // ⭐ PER CLIENT

    char filename[100];
    long long file_size;

    SSL_read(ssl, filename, sizeof(filename));
    SSL_read(ssl, &file_size, sizeof(file_size));

    FILE *fp = fopen(filename, "wb");

    printf("\nReceiving: %s (%lld bytes)\n", filename, file_size);

    Packet buffer[WINDOW_SIZE];
    int received[WINDOW_SIZE] = {0};
    int base = 0;

    while (1) {
        Packet p;
        int bytes = SSL_read(ssl, &p, sizeof(p));
        if (bytes <= 0) break;

        encrypt_decrypt(p.data, p.size);

        if (p.seq_no % 3 == 0 && loss_done[p.seq_no] == 0) {
            loss_done[p.seq_no] = 1;
            printf("Packet %d LOST\n", p.seq_no);
            continue;
        }

        printf("Packet %d RECEIVED\n", p.seq_no);

        int idx = p.seq_no % WINDOW_SIZE;

        if (!received[idx]) {
            buffer[idx] = p;
            received[idx] = 1;
        }

        Ack ack;
        ack.ack_no = p.seq_no;
        SSL_write(ssl, &ack, sizeof(ack));

        printf("ACK %d SENT\n", p.seq_no);

        while (received[base % WINDOW_SIZE]) {
            Packet wp = buffer[base % WINDOW_SIZE];
            fwrite(wp.data, 1, wp.size, fp);
            received[base % WINDOW_SIZE] = 0;
            base++;
        }

        usleep(100000); // ⭐ IMPORTANT FIX
    }

    fclose(fp);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    pthread_exit(NULL);
}

int main() {

    SSL_library_init();
    ctx = SSL_CTX_new(TLS_server_method());

    SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, 5);

    printf("🔥 Server Started (Multi-client + SSL)\n");

    while (1) {
        int client = accept(sock, NULL, NULL);

        printf("New client connected\n");

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0) continue;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, ssl);
        pthread_detach(tid);
    }
}