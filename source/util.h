#ifndef UTIL_H
#define UTIL_H

#include <3ds.h>

#define GENERR_OK 		0;
#define GENERR_ALLOC 	1;
#define GENERR_UNKNOWN 	255;

//#define safefree (in) (if ((in) != NULL) free(in))

typedef struct {
	void *res;
	u8 err;
} genericres;

typedef struct {
	genericres res;
	u16 len;
	u8 err;
} splitres;

char recursiveFree(char**, unsigned int);

//u8 safefree(void*);

#endif