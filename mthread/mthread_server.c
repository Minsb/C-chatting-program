#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define CLIENT_SIZE 256 //num of max user number
#define NAME_SIZE 20

char buf[BUF_SIZE];

void* handle_clnt(void* arg);
void send_msg(char* msg, int len, int clnt_sock);
void error_handling(char* message);

int clnt_cnt = 0;
int clnt_socks[CLIENT_SIZE];
pthread_mutex_t mutx;

char* userlist[CLIENT_SIZE];

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;

	pthread_t t_id;


	int fd_max, str_len, fd_num, i;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	pthread_mutex_init(&mutx, NULL);

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
	{
		error_handling("bind() error");
	}
	if (listen(serv_sock, 256) == -1)
	{
		error_handling("listen() error");
	}

	while (1)
	{
		adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);

		pthread_mutex_lock(&mutx);
		//critical section
		clnt_socks[clnt_cnt++] = clnt_sock;

		//receive name
		recv(clnt_sock, buf, NAME_SIZE, 0);


		//insert name

		userlist[clnt_cnt - 1] = malloc(sizeof(char) * NAME_SIZE);

		//don't allow same username
		if (clnt_cnt != 0) {
			for (int i = 0; i < clnt_cnt; i++)
			{
				if (strcmp(userlist[i], buf) == 0)
				{
					close(clnt_sock);
				}
			}
		}

		strcpy(userlist[clnt_cnt - 1], buf);

		pthread_mutex_unlock(&mutx);

		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("User %s has joined chat\n", userlist[clnt_cnt - 1]);
		
		memset(buf, '\0', sizeof(buf));
	}
	close(serv_sock);
	return 0;
}
void* handle_clnt(void* arg)
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE];

	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
	{
		send_msg(msg, str_len, clnt_sock);
	}
	for (int j = 0; j < clnt_cnt; j++)
	{
		if (clnt_sock == clnt_socks[j])
		{
			printf("User %s has left the chat\n", userlist[j]);
			break;
		}
	}

	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++) //remove disconnected client
	{
		if (clnt_sock == clnt_socks[i])
		{
			while (i < clnt_cnt - 1)
			{
				clnt_socks[i] = clnt_socks[i + 1];
				strcpy(userlist[i], userlist[i + 1]);
				i++;
			}
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	
	close(clnt_sock);
	return NULL;
}

void send_msg(char* msg, int len, int clnt_sock)
{
	int i;
	char copy[BUF_SIZE];
	char command[NAME_SIZE];

	char* ptr = strchr(msg, ' ');

	strcpy(copy, msg);

	//get command 
	char* token = strtok(copy, " ");

	strcpy(command, token);

	while (token != NULL)
	{
		token = strtok(NULL, " ");
	}

	pthread_mutex_lock(&mutx);

	//send all
	if (strcmp(command, "@all") == 0)
	{
		for (i = 0; i < clnt_cnt; i++)
			write(clnt_socks[i], ptr, len);
	}
	//send to another user
	else if (command[0] == '@')
	{
		int targetfound = 0;
		for (int i = 0; i < clnt_cnt; i++)
		{
			if (strcmp(command, userlist[i]) == 0)
			{
				write(clnt_socks[i], ptr, len);
				targetfound = 1;
			}
		}
		if (targetfound == 0)
		{
			char notfound[24] = "Target user not found!\n";
			write(clnt_sock, notfound, 24);
		}
	}
	//error
	else
	{
		printf("input error occur\n");
	}
	pthread_mutex_unlock(&mutx);
}
void error_handling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}