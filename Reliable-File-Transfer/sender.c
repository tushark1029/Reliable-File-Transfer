#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/time.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define WINDOW_SIZE 4
#define TIMEOUT 2

typedef struct {
    int seq_no;
    int size;
    char data[BUFFER_SIZE];
} Packet;

typedef struct {
    int ack_no;
} Ack;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

void encrypt_decrypt(char *data, int size) {
    for (int i = 0; i < size; i++)
        data[i] ^= 'K';
}

int main() {

    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);

    char ip[50];
    printf("Enter receiver IP: ");
    scanf("%s", ip);

    inet_pton(AF_INET, ip, &serv.sin_addr);
    connect(sock, (struct sockaddr *)&serv, sizeof(serv));

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);
    SSL_connect(ssl);

    char filename[100];
    printf("Enter file: ");
    scanf("%s", filename);

    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("File not found\n");
        return 0;
    }

    long long file_size;
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    SSL_write(ssl, filename, sizeof(filename));
    SSL_write(ssl, &file_size, sizeof(file_size));

    Packet window[WINDOW_SIZE];
    int acked[WINDOW_SIZE] = {0};
    double send_time[WINDOW_SIZE];
    int loss_done[1000] = {0};

    int base = 0, next = 0;

    while (1) {

        // SEND WINDOW
        while (next < base + WINDOW_SIZE) {
            Packet p;
            p.seq_no = next;
            p.size = fread(p.data, 1, BUFFER_SIZE, fp);

            if (p.size <= 0) break;

            encrypt_decrypt(p.data, p.size);
            window[next % WINDOW_SIZE] = p;

            if (p.seq_no % 3 == 0 && loss_done[p.seq_no] == 0) {
                loss_done[p.seq_no] = 1;
                printf("Packet %d LOST\n", p.seq_no);
            } else {
                SSL_write(ssl, &p, sizeof(p));
                printf("Packet %d SENT\n", p.seq_no);
            }

            send_time[next % WINDOW_SIZE] = get_time();
            next++;
        }

        // WAIT FOR ACK (NON-BLOCKING)
        fd_set readfds;
        struct timeval tv;

        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);

        tv.tv_sec = 0;
        tv.tv_usec = 200000;

        int activity = select(sock + 1, &readfds, NULL, NULL, &tv);

        if (activity > 0 && FD_ISSET(sock, &readfds)) {
            Ack ack;
            if (SSL_read(ssl, &ack, sizeof(ack)) > 0) {
                printf("ACK %d RECEIVED\n", ack.ack_no);
                acked[ack.ack_no % WINDOW_SIZE] = 1;
            }
        }

        // RETRANSMISSION
        for (int i = base; i < next; i++) {
            int idx = i % WINDOW_SIZE;

            if (!acked[idx] && (get_time() - send_time[idx]) > TIMEOUT) {
                SSL_write(ssl, &window[idx], sizeof(Packet));
                printf("Packet %d RETRANSMITTED\n", i);
                send_time[idx] = get_time();
            }
        }

        // SLIDE WINDOW
        while (acked[base % WINDOW_SIZE]) {
            acked[base % WINDOW_SIZE] = 0;
            base++;
        }

        if (feof(fp) && base == next) break;

        usleep(200000); // ⭐ IMPORTANT FIX
    }

    printf("File sent successfully\n");

    fclose(fp);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
}