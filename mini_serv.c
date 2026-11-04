#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

int count = 0, max_fd = 0;

int client_id[65556];
char *msg[1024];

fd_set rfds, wfds, afds;

char buf_read[1001], buf_write[42];


void	fatal_error() {

	write(2, "Fatal error\n", 12);
	exit(1);
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				fatal_error();
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		fatal_error();
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

void	broadcast(int author, char* msg) {

	for(int fd = 0; fd <= max_fd; fd++) {

		if (FD_ISSET(fd, &wfds) && fd != author) {

			send(fd, msg, strlen(msg), 0);
		}
	}
}

void	add_client(int fd) {

	max_fd = fd > max_fd ? fd : max_fd;
	client_id[fd] = count++;
	FD_SET(fd, &afds);
	msg[fd] = NULL;

	sprintf(buf_write, "server: client %d just arrived\n", client_id[fd]);
	broadcast(fd, buf_write);

}

void	remove_client(int fd) {

	sprintf(buf_write, "server: client %d just left\n", client_id[fd]);
	broadcast(fd, buf_write);

	FD_CLR(fd, &afds);
	if(msg[fd]) {
		free(msg[fd]);
		msg[fd] = NULL;
	}
	close(fd);

}

void	send_msg(int fd) {

	char *message;

	while(extract_message(&msg[fd], &message)) {

		sprintf(buf_write, "client %d: ", client_id[fd]);
		broadcast(fd, buf_write);
		broadcast(fd, message);
		free(message);
	}

}

int main(int ac, char **av) {

	if(ac != 2) {

		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	FD_ZERO(&afds);
	uint16_t port = atoi(av[1]);

	// socket create and verification
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
		fatal_error();
	FD_SET(sockfd, &afds);
	max_fd = sockfd;


	struct sockaddr_in servaddr, cli;
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(port);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
		close(sockfd);
		fatal_error();
	}
	if (listen(sockfd, 4096) != 0) {
		close(sockfd);
		fatal_error();
	}

	while(1) {

		rfds = wfds = afds;

		if (select(max_fd + 1, &rfds, &wfds, NULL, NULL) < 0){
			fatal_error();
		}

		for(int fd = 0; fd <= max_fd; fd++) {

			if (!FD_ISSET(fd, &rfds))
				continue;


			if (fd == sockfd) {

				socklen_t len = sizeof(cli);
				int client_fd = accept(sockfd, (struct sockaddr *)&cli, &len);
				if (client_fd >= 0) {

					add_client(client_fd);
					break;
				}

			}

			else {

				int bytes_read = recv(fd, buf_read, 1000, 0);
				if (bytes_read <= 0) {

					remove_client(fd);
					break;
				}
				buf_read[bytes_read] = 0;
				msg[fd] = str_join(msg[fd], buf_read);
				send_msg(fd);
			}
		}
	}
}