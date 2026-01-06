#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, n, fd, clilength;
    char buff[BUFFER_SIZE], fc[BUFFER_SIZE], filename[100], new_filename[100];
    struct sockaddr_in servaddr, cliaddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(5106);

    // Bind socket
    printf("Server listening at port 5106\n");
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    // Listen for incoming connections
    if (listen(sockfd, 2) == -1) {
        perror("Listen failed");
        close(sockfd);
        return 1;
    }

    clilength = sizeof(cliaddr);

    while (1) {
        // Accept incoming connection
        newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilength);
        if (newsockfd == -1) {
            perror("Accept failed");
            close(sockfd);
            return 1;
        }

        // Read data from the client
        if ((n = read(newsockfd, buff, sizeof(buff))) == -1) {
            perror("Read failed");
            close(newsockfd);
            continue;
        }

        // Process the received data
        if (strstr(buff, "get") != NULL) {
            sscanf(buff, "get %99[^\n]", filename);
            printf("Reading file content from server\n");
            fd = open(filename, O_RDONLY);
            if (fd == -1) {
                perror("File open failed");
                close(newsockfd);
                continue;
            }
            while ((n = read(fd, fc, sizeof(fc))) > 0) {
                if (write(newsockfd, fc, n) == -1) {
                    perror("Write failed");
                    close(fd);
                    close(newsockfd);
                    break;
                }
            }
            close(fd);
            printf("End if\n");
        } else if (strstr(buff, "upload") != NULL) {
            sscanf(buff, "upload %99[^\n]", filename);
            printf("Upload\n");
            strcpy(new_filename, "new");
            strcat(new_filename, filename);
            fd = open(new_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd == -1) {
                perror("File open failed");
                close(newsockfd);
                continue;
            }
            while ((n = read(newsockfd, buff, sizeof(buff))) > 0) {
                if (write(fd, buff, n) == -1) {
                    perror("Write failed");
                    close(fd);
                    close(newsockfd);
                    break;
                }
            }
            close(fd);
        } else {
            printf("Invalid operation.\n");
        }
        close(newsockfd);
    }

    close(sockfd);
    return 0;
}

