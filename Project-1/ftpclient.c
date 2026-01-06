#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd, n, fd, port;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE], filename[100], new_filename[100];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    servaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1) {
        perror("Invalid address");
        return 1;
    }
    port = atoi(argv[2]);
    servaddr.sin_port = htons(port);

    if (connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("Connection failed");
        return 1;
    }

    printf("Enter Operation: ");
    fgets(buffer, sizeof(buffer), stdin);
    write(sockfd, buffer, strlen(buffer));

    if (strstr(buffer, "get") != NULL) {
        sscanf(buffer, "get %99[^\n]", filename);
        printf("Reading\n");
        strcpy(new_filename, "new");
        strcat(new_filename, filename);
        fd = open(new_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            perror("File open failed");
            return 1;
        }
        while ((n = read(sockfd, buffer, sizeof(buffer))) > 0) {
            if (write(fd, buffer, n) == -1) {
                perror("File write failed");
                return 1;
            } else if (n < BUFFER_SIZE) {
                break;
            }
        }
        close(fd);
        printf("File received successfully.\n");
    } else if (strstr(buffer, "upload") != NULL) {
        sscanf(buffer, "upload %99[^\n]", filename);
        printf("Uploading file content to server\n");
        fd = open(filename, O_RDONLY);
        while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
            write(sockfd, buffer, n);
        }
        close(fd);
    } else {
        printf("Invalid operation.\n");
    }

    close(sockfd);
    return 0;
}

