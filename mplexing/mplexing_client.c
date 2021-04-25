#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void error_handling(char* message);
void read_routine(int sock, char* buf);
void write_routine(int sock, char* buf);

char name[NAME_SIZE] = "[NULL]";
char buf[BUF_SIZE];

int main(int argc, char* argv[]) {
    int tcp_socket = socket(PF_INET, SOCK_STREAM, 0);;
    pid_t pid;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_adr;

    //client socket
    sprintf(name, "[%s]", argv[3]);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    //connect to server
    if (connect(tcp_socket, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error!");

    //fork
    pid = fork();
    if (pid == 0) //child
        write_routine(tcp_socket, buf);
    else
        read_routine(tcp_socket, buf);
    usleep(15);
    close(tcp_socket);
    return 0;
}


void write_routine(int socket char* buffer) {
    char msg[NAME_SIZE + BUF_SIZE] = "";
    while (true)
    {
        fgets(buf, BUF_SIZE, stdin);
        //half close
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) {
            int shutdown(sock, SHUT_WR);
            return;
        }
        sprintf_s(msg, sizeof(msg), "%s %s", name, buffer);
        write(sock, msg, strlen(msg));
    }
}

void read_routine(int socket, char* buffer) {
    char msg[NAME_SIZE + BUF_SIZE];
    while (true) {
        int str_len = read(socket, msg, NAME_SIZE + BUF_SIZE);
        if (str_len == 0)
            return;
        msg[str_len] = 0;
        fputs(msg, stdout);
    }
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
