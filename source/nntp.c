#include "nntp.h"

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

#include "util.h"

nntpcon nntpinit(char *server, u16 port){
	struct sockaddr_in addr;
	int sock;
	struct hostent *ip_ent;
	nntpcon con;
	//struct pollfd pfd[1];
	
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
	//addr.sin_addr.s_addr = inet_addr("10.0.0.205");
	printf("connecting\n");
	
	//pfd[0].fd = sock;
	//pfd[0].events = POLLIN;
		
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		con.err = NNTPERR_CON_FAILURE;
		con.socketfd = -1;
		return con;
	}
	
	printf("connected\n");
	con.err = NNTPERR_OK;
	con.socketfd = sock;
	return con;
	
}

nntpres nntp_custom_command(char *msg, nntpcon con){
	nntpres out;
	out.err = NNTPERR_UNKNOWN;
	out.contents = NULL;
	char *buf = malloc(128);
	u32 size = 0;
	u32 buf_size = 126;
	u32 last_read_size = 0;
	
	out.err = send(con.socketfd, msg, strlen(msg), 0);
	
	while ((last_read_size = recv(con.socketfd, buf, 126, 0))  > 0){
		size += last_read_size;
		buf_size = size + 126;
		buf = realloc(buf, buf_size);
	}
	
	out.contents = malloc(size + 1);
	strcpy(out.contents, buf);
	free(buf);
	if (out.contents == NULL){
		out.err = NNTPERR_ALLOC;
		return out;
	}
	
	out.err = NNTPERR_OK;
	return out;
	
}

nntpgroups nntp_get_groups(nntpcon con){
	nntpgroups out;
	out.err = NNTPERR_UNKNOWN;
	out.groups = NULL;
	char *buf = malloc(4096);
	int size = 0;
	int buf_size = 4094;
	int last_read_size = 0;
	struct pollfd pfd[1];
	memset(pfd, 0, sizeof(pfd));
	int pollret;
	const u16 read_size = 65535;
	
	
	// Block to send
	//fcntl(con.socketfd, F_SETFL, fcntl(con.socketfd, F_GETFL, 0) & ~ O_NONBLOCK);
	
	if(fcntl(con.socketfd, F_SETFL, fcntl(con.socketfd, F_GETFL, 0) | O_NONBLOCK) == -1) {
		/*close(sock);
        gfxExit();
        socExit();
        return 0;*/
		out.err = NNTPERR_SOCKCONF;
		free(buf);
		return out;
	}
	
	out.err = send(con.socketfd, "LIST\n", strlen("LIST\n"), 0);
	
	//sleep(1);
	
	// Disable blocking
	//fcntl(con.socketfd, F_SETFL, fcntl(con.socketfd, F_GETFL, 0) | O_NONBLOCK);
	pfd[0].fd = con.socketfd;
	pfd[0].events = POLLIN;
	printf("Polling\n");
	
	while ((last_read_size = recv(con.socketfd, buf, read_size, MSG_DONTWAIT)) != 0){
		if (last_read_size == -1){
			if (errno == 11) continue;
			printf("Read error %d\n", errno);
			out.err = NNTPERR_READ;
			free(buf);
			return out;
		}
		
		//if (last_read_size == sizeof(buf)) break;
		
		size += last_read_size;
		printf("Size: %d\n", size);
		buf_size = size + read_size;
		buf = realloc(buf, buf_size);
		if (buf == NULL){
			out.err = NNTPERR_ALLOC;
			out.groups = NULL;
		}
		//printf("%s\n", buf);
	}	
	
	/*
	while ((pollret = poll(pfd, pfd[0].fd + 1, -1)) != 0 ){
		printf("%d\n", pollret);
		printf("Poll start\n");
		if (pfd[0].revents & POLLIN){
			
			last_read_size = recv(con.socketfd, buf, 0x1000, 0);
			if (last_read_size == -1){
				printf("Read error %d\n", errno);
				out.err = NNTPERR_READ;
				return out;
			}
			
			//if (last_read_size == sizeof(buf)) break;
			
			size += last_read_size;
			printf("Size: %d\n", size);
			buf_size = size * 2;
			buf = realloc(buf, buf_size);
			if (buf == NULL){
				out.err = NNTPERR_ALLOC;
				out.groups = NULL;
			}
		}
		printf("End poll\n");
	}*/
	//printf("%d", pollret);
	printf("Fin\n");
	
	/*out.contents = malloc(size + 1);
	strcpy(out.contents, buf);
	free(buf);
	if (out.contents == NULL){
		out.err = NNTPERR_ALLOC;
		return out;
	}*/
	/*
	splitres res = util_split_by_newline(buf);
	out.size = res.len;
	out.groups = malloc(sizeof(char *) * (out.size + 1));
	memcpy(out.groups, res.res.res, sizeof(char *) * out.size);
	
	free(buf);
	free(res.res.res);*/
	/*
	for (u32 i = 0; i < out.size - 1; i++){
		char *pos;
		pos = strchr(out.groups[i], ' ');
		out.groups[i][(u8) *pos] = '\0';
	}*/
	free(buf);
	out.err = NNTPERR_OK;
	return out;
}