#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 4443
#define BUFFER_SIZE 4096

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX *create_context() {
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

int main() {
    int sock;
    struct sockaddr_in server_addr;

    init_openssl();
    SSL_CTX *ctx = create_context();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("Connection failed");
        close(sock);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        close(sock);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        cleanup_openssl();
        exit(EXIT_FAILURE);
    }

    printf("Connected with %s encryption\n", SSL_get_cipher(ssl));

    char buffer[BUFFER_SIZE];
    int bytes;

    while (1) {
        printf("Enter Message: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        if (strncasecmp(buffer, "bye", 3) == 0 && (buffer[3] == '\n' || buffer[3] == '\0')) {
            SSL_write(ssl, buffer, strlen(buffer));
            printf("Client initiated disconnect.\n");
            break;
        }

        SSL_write(ssl, buffer, strlen(buffer));

        bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) break;

        buffer[bytes] = 0;
        printf("Server: %s", buffer);

        if (strncasecmp(buffer, "bye", 3) == 0 && (buffer[3] == '\n' || buffer[3] == '\0')) {
            printf("Server initiated disconnect.\n");
            break;
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();

    printf("Client terminated.\n");
    return 0;
}