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
	
	u32 workingsize = sizeof(char) * 65534;
	u32 workinginit = workingsize;
	char *working = malloc(workingsize);
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
		
		//memset(working, 0, (workingsize - 1));
		recv_res = recv(con.socketfd, working, (workingsize - 1), 0);
		if (recv_res == -1){
			printf("Read error\n");
			out.groups = NULL;
			out.err = NNTPERR_READ;
			out.errcode = errno;
			return out;
		}
		
		working[recv_res] = '\0';
		if ((bufsize + recv_res) >= bufsize) {
			last_buf = buf;
			buf = realloc(buf, bufsize + recv_res);
			if (buf == NULL){
				out.err = NNTPERR_ALLOC;
				free(working);
				free(last_buf);
				return out;
			}
			bufsize += recv_res + 1;
			printf("bufsize: %lld\n", bufsize);
			
		}
		
		printf("recv: %d\n", recv_res);
		
		strcat(buf, working);	
		
		bufpos += recv_res;
	}
	
	printf("Got\n");
	sleep(2);
	//free(working);
	//memset(working, 0, strlen(working));
	buf[bufpos] = '\0';
	backup_buf = malloc(bufsize + 1);
	backup_buf_size = bufsize;
	
	wpos = 0;
	
	strncpy(backup_buf, buf, backup_buf_size);
	//backup_buf[backup_buf_size - 1] = '\0';
	out.len = 0;
	u32 endpos;
	last_buf = malloc(bufsize);
	while ((endpos = strcspn(buf, "\n")) != strlen(buf)){
		
		strncpy(working, buf, workingsize);
		printf("Copied buf to working\n");
		sleep(3);
		working[endpos - 1] = '\0';
		printf("Modified working\n");
		sleep(3);
		u16 spacepos;
		strncpy(out.groups[out.len], working, workingsize);
		printf("Copied working to groups\n");
		sleep(3);
		/*if ((spacepos = strcspn(working, " ")) != strlen(working)){
			out.len++;
			working[spacepos] = '\0';
			strcpy(out.groups[out.len - 1], working);
			continue;
		}*/
		
		
		last_buf = buf;
		buf = memmove(buf, buf + endpos, bufsize - endpos);
		printf("Resized buf\n");
		sleep(3);
	}
	
	/*
	for (u32 i = 0; i < bufpos; i++){
		printf("wpos: %ld\ni: %ld\n", wpos, i);

		//sleep(1);
		if (buf[i] == '\n'){
			sleep(1);
			working[++wpos] = '\0';
			//fflush(stdout);
			//out.groups = malloc(sizeof(char*));
			out.groups[grouppos] = malloc(workingsize);
			if (out.groups[grouppos] == NULL){
				out.err = NNTPERR_ALLOC;
				free(buf);
				free(working);
				return out;
			}
			out.len++;
			grouppos++;
			wpos = 0;
			strcpy(out.groups[grouppos], working);

			//out.groups[linepos] = realloc(out.groups[linepos], bufpos);
			//memset(working, 0, workingsize);
			
			if (out.len % 75 == 0) {
				printf(".");
				fflush(stdout);
			}
			
			printf("\n");
			continue;
		}
		
		working[wpos++] = buf[i];
	}*/
	
	free(buf);
	free(working);
	out.err = NNTPERR_OK;
	return out;
}