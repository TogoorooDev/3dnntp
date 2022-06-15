#include "util.h"

#include <string.h>
#include <stdlib.h>

char recursiveFree(char **in, unsigned int len){
	if (in != NULL){
		for(u32 i = 0; i < len; i++){
			if (in[i] != NULL) free(in[i]);
		}
		free(in);
		return 0;
	}
	return -1;
}