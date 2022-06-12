#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <poll.h>
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

#include <3ds.h>

#include "nntp.h"
#include "util.h"

nntpcon nntpinit(char *server, u16 port){
	struct sockaddr_in addr;
	int sock, pollres, recv_res;
	struct hostent *ip_ent;
	nntpcon con;
	struct pollfd watchlist[1];
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	ip_ent = gethostbyname(server);
	if (ip_ent == NULL){
		con.err = NNTPERR_RESOLUTION;
		con.socketfd = -1;
		return con;
	}
	
	memcpy(&addr.sin_addr, ip_ent->h_addr_list[0], ip_ent->h_length);
	printf("connecting\n");
	
	char *discard = malloc(16);
	
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		con.err = NNTPERR_CON_FAILURE;
		con.socketfd = -1;
		return con;
	}
	
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
	
	watchlist[0].fd = sock;
	watchlist[0].events = POLLIN;
	
	// Flush buffer
	while ((pollres = poll(watchlist, 1, 2500)) != 0){
		if (pollres == -1){
			con.err = NNTPERR_POLL;
			return con;
		}
		if ((recv_res = recv(sock, discard, 1, 0)) != 0){
			if (recv_res == -1){
				con.err = NNTPERR_READ;
				return con;
			}
			
		}	
	}
	
	free(discard);
	
	printf("connected\n");
	con.err = NNTPERR_OK;
	con.socketfd = sock;
	return con;
	
}

// nntpres nntp_custom_command(char *msg, nntpcon con){
	// nntpres out;
	// out.err = NNTPERR_UNKNOWN;
	// out.contents = NULL;
	// char *buf = malloc(128);
	// u32 size = 0;
	// u32 buf_size = 126;
	// u32 last_read_size = 0;
	
	// out.err = send(con.socketfd, msg, strlen(msg), 0);
	
	// while ((last_read_size = recv(con.socketfd, buf, 126, 0))  > 0){
		// size += last_read_size;
		// buf_size = size + 126;
		// buf = realloc(buf, buf_size);
	// }
	
	// out.contents = malloc(size + 1);
	// strcpy(out.contents, buf);
	// free(buf);
	// if (out.contents == NULL){
		// out.err = NNTPERR_ALLOC;
		// return out;
	// }
	
	// out.err = NNTPERR_OK;
	// return out;
	
// }

nntpgroups nntp_get_groups(nntpcon con){
	nntpgroups out;
	out.err = NNTPERR_UNKNOWN;
	out.groups = NULL;
	out.len = 0;
	char *buf = calloc(65535, sizeof(char));
	u16 bufpos = 0;
	u16 linepos = 0;
	char *temp = calloc(2, sizeof(char));
	int recv_res = 0;
	int pollres;
	struct pollfd watchlist[1];
	watchlist[0].fd = con.socketfd;
	watchlist[0].events = POLLIN;
	
	fcntl(con.socketfd, F_SETFL, fcntl(con.socketfd, F_GETFL, 0) & ~O_NONBLOCK);
	
	send(con.socketfd, "LIST\r\n", strlen("LIST\r\n") + 1, 0);
	
	fcntl(con.socketfd, F_SETFL, fcntl(con.socketfd, F_GETFL, 0) | O_NONBLOCK);
	
	while ((pollres = poll(watchlist, 1, 5000)) != 0){
		if (pollres == -1){
			printf("Poll error\n");
			out.err = NNTPERR_POLL;
			out.groups = NULL;
			return out;
		}
		if ((recv_res = recv(con.socketfd, temp, 1, 0)) != 0){
			if (recv_res == -1){
				printf("Read error\n");
				out.groups = NULL;
				out.err = NNTPERR_READ;
				out.errcode = errno;
				return out;
			}
			
			if (temp[0] == '\n') {
				buf[++bufpos] = '\0';
				out.groups = malloc(sizeof(char*));
				out.groups[linepos] = malloc(sizeof(char) * 65535);
				out.len++;
				linepos++;
				bufpos = 0;
				//out.groups[linepos] = realloc(out.groups[linepos], bufpos);
				memset(buf, 0, sizeof(char) * 65535);
				continue;
			}
			
			printf("%c\n", temp[0]);

			buf[bufpos] = temp[0];
			bufpos++;
		}	
	}

	free(temp);
	free(buf);
	out.err = NNTPERR_OK;
	return out;
}