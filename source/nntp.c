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
//#include "util.h"

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

/* nntpres nntp_custom_command(char *msg, nntpcon con){
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
	
// } */


nntpgroups nntp_get_groups(nntpcon con){
	nntpgroups out;
	out.err = NNTPERR_UNKNOWN;
	out.groups = NULL;
	out.len = 0;
	
	u32 workingsize;
	char *working;
	u32 wpos = 0;
	
	u64 bufsize = sizeof(char) * 65534;
	u32 bufinit = bufsize;
	char *buf = malloc(bufsize);
	u32 bufpos = 0;
	char *last_buf;
	
	u64 backup_buf_size;
	char *backup_buf;
	
	u32 groupsize = sizeof(char*) * 65534;
	u32 grouppos = 0;
	
	int recv_res = 0;
	
	int pollres;
	struct pollfd watchlist[1];
	watchlist[0].fd = con.socketfd;
	watchlist[0].events = POLLIN;
	
	fcntl(con.socketfd, F_SETFL, fcntl(con.socketfd, F_GETFL, 0) & ~O_NONBLOCK);
	
	send(con.socketfd, "LIST\r\n", strlen("LIST\r\n") + 1, 0);
	
	fcntl(con.socketfd, F_SETFL, fcntl(con.socketfd, F_GETFL, 0) | O_NONBLOCK);
	
	out.groups = malloc(groupsize);
	
	while ((pollres = poll(watchlist, 1, 5000)) != 0){
		if (pollres == -1){
			printf("Poll error\n");
			out.err = NNTPERR_POLL;
			out.groups = NULL;
			return out;
		}
		last_buf = buf;
		
		if ((bufsize - bufpos) < 8192){
			bufsize += 8192;
			buf = realloc(buf, bufsize);
			if (buf == NULL){
				out.err = NNTPERR_ALLOC;
				free(last_buf);
				free(out.groups);
				out.groups = NULL;
				
				printf(".");
				fflush(stdout);
				
				return out;
			}
		}
		
		recv_res = recv(con.socketfd, buf + bufpos, bufsize - bufpos, 0);
		if (recv_res == -1){
			printf("Read error\n");
			out.groups = NULL;
			out.err = NNTPERR_READ;
			out.errcode = errno;
			return out;
		}
		bufpos += recv_res;
	}
	
	printf("Got\n");
	sleep(2);
	
	buf[bufpos] = '\0';
	backup_buf = malloc(bufsize + 1);
	backup_buf_size = bufsize;
	
	wpos = 0;
	
	strncpy(backup_buf, buf, backup_buf_size);
	//backup_buf[backup_buf_size - 1] = '\0';
	out.len = 0;
	u32 endpos;
	
	working = strtok(buf, "\n");
	while (working != NULL){
		out.groups[wpos] = malloc(strlen(working) + 1);
		working[strlen(working)] = '\0';
		last_buf = memcpy(out.groups[wpos], working, strlen(working) + 1);
		if (last_buf == NULL){
			out.err = NNTPERR_MEM;
			return out;
		}
		
		wpos++;
		working = strtok(NULL, "\n");
	}

	out.len = wpos;

	free(buf);
	//free(thaiboy);
	out.err = NNTPERR_OK;
	return out;
}