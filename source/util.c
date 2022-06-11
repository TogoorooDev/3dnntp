#include "util.h"

#include <string.h>
#include <stdlib.h>

#include <3ds.h>

splitres util_split_by_newline(char *in){
	char *token;
	char **arr = malloc(128 * sizeof(char*));
	u16 counter = 0;
	u16 bufsize = 0;
	splitres out;
	out.err = GENERR_UNKNOWN;
	out.res.res = NULL;
	out.len = 0;
	
	token = strtok(in, "\n");
	while (token != NULL){
		u16 size = sizeof(char) * (strlen(token) + 1);
		bufsize += sizeof(char*);
		arr[counter] = malloc(size);
		if (arr[counter] == NULL){
			out.err = GENERR_ALLOC;
			return out;
		}
		strcpy(arr[counter], token);
		counter++;
		token = strtok(NULL, "\n");
	}
	
	out.len = counter;
	out.res.res = malloc(bufsize + 1);
	if (out.res.res == NULL){
		out.err = GENERR_ALLOC;
		return out;
	}
	memcpy(arr, out.res.res, bufsize);
	free(arr);
	return out;
}


















































/*
splitres util_split_by_newline(char *in){
	int insize = strlen(in);
	char *token;
	char **out = malloc(insize * sizeof(char *));
	u32 counter = 0;
	splitres ret;
	ret.res.err = GENERR_UNKNOWN;
	u32 size = 0;
	
	token = strtok(in, "\n");
	while (token != NULL){
		out[counter] = malloc(strlen(token) * sizeof(char) + 1);
		if (out[counter] == NULL){
			free(out);
			free(in);
			ret.err = GENERR_ALLOC;
			return ret;
		}
		strcpy(out[counter], token);
		counter++;
		token = strtok(NULL, "\n");
	}
	ret.len = counter + 1;
	out = realloc(out, counter * sizeof(char *));
	if (out == NULL){
		free(in);
		ret.err = GENERR_ALLOC;
		return ret;
	}
	ret.res.res = malloc(counter * sizeof(char *));
	if (ret.res.res == NULL){
		free(out);
		free(in);
		ret.err = GENERR_ALLOC;
		return ret;
	}
	memcpy(ret.res.res, out, ret.len * sizeof(char*));
	free(in);
	free(out);
	ret.err = GENERR_OK;
	return ret;
}
*/