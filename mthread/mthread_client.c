#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void error_handling(char* message);
void* recv_msg(void* socket);
void* send_msg(void* socket);

char name[NAME_SIZE] = "DEFAULT";
char buf[BUF_SIZE];
char command[NAME_SIZE];
int command_len;

int main(int argc, char* argv[]) {
    int sock;
    pthread_t snd_thread, rcv_thread;
    struct sockaddr_in serv_adr;
    void* thread_return;

    if (argc != 4) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    //client socket
    sprintf(name, "@%s", argv[3]);
    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error!");

    //send name
    send(sock, name, strlen(name), 0);

    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&snd_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}


void* send_msg(void* arg) {
    int sock = *((int*)arg);
    char msg[NAME_SIZE + BUF_SIZE];
    
    char temp[NAME_SIZE + BUF_SIZE];
 
    while (1)
    {
        //get command
        scanf("%s", command);

        //quit
        if (!strcmp(command, "q\0") || !strcmp(command, "Q\0")) {
            write(sock, "", 0);
            close(sock);
            exit(0);
        }

        //get left message from buffer
        fgets(buf, BUF_SIZE, stdin);
        sprintf(msg, "%s %s :%s\n" ,command ,name, buf);
        write(sock, msg, strlen(msg));
    }
}

void* recv_msg(void* arg) {
    int sock = *((int*)arg);
    char msg[NAME_SIZE + BUF_SIZE];
    int str_len;
    while (1) {
        str_len = read(sock, msg, NAME_SIZE + BUF_SIZE - 1);
        if (str_len == -1)
            return (void*)-1;
        msg[str_len] = 0;
        fputs(msg, stdout);
    }
    return NULL;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
