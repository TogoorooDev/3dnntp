#ifndef NNTP_H
#define NNTP_H

#include <3ds.h>

#define NNTPERR_OK				0
#define NNTPERR_CON_FAILURE 	1
#define NNTPERR_INVALID_ADDR	2
#define NNTPERR_ALLOC			3
#define NNTPERR_RESOLUTION		4
#define NNTPERR_TIMEOUT			5
#define NNTPERR_READ			6
#define NNTPERR_SOCKCONF		7
#define NNTPERR_UNKNOWN 		255

typedef struct{
	int socketfd;
	u8 err;
} nntpcon;

typedef struct{
	u32 size;
	char *contents;
	u8 err;
} nntpres;

typedef struct {
	u32 size;
	char **groups;
	char *lgroup;
	u8 err;
} nntpgroups;

nntpcon nntpinit(char*, u16);
nntpres nntp_custom_command(char*, nntpcon);
nntpgroups nntp_get_groups(nntpcon con);

//#include "nntp.c"

#endif