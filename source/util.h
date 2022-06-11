#ifndef UTIL_H
#define UTIL_H

#include <3ds.h>

#define GENERR_OK 		0;
#define GENERR_ALLOC 	1;
#define GENERR_UNKNOWN 	255;

typedef struct {
	void *res;
	u8 err;
} genericres;

typedef struct {
	genericres res;
	u16 len;
	u8 err;
} splitres;

splitres util_split_by_newline(char*);

#endif