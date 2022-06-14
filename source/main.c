//#include <citro2d.h>

/* Integer is 16 bit */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <malloc.h>

#include <3ds.h>

#include "nntp.h"

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

#define SOC_BUF_SIZE 	0x100000
#define SOC_BUF_ALIGN 	0x1000

static u32 *SOC_buf = NULL;

void socExitWrapper(){
	printf("exiting...\n");
	socExit();
}

int main(){
	int ret;

	gfxInitDefault();
	// Register gfxExit() and socExit() to be run when app quits
	atexit(gfxExit);
	
	PrintConsole topScreen, bottomScreen;

	// Initialize console for both screen using the two different PrintConsole we have defined
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	
	consoleSelect(&topScreen);
	printf("3DNNTP v0.0.1\n");
	
	SOC_buf = memalign(SOC_BUF_ALIGN, SOC_BUF_SIZE);
	if (SOC_buf == NULL){
		printf("Allocation Error\nPress B to exit\n");
		while (aptMainLoop()){
			gspWaitForVBlank();
			hidScanInput();
			u32 kDown = hidKeysDown();
			if (kDown & KEY_B) {
				socExitWrapper();
				gfxExit();
				exit(1);
			}
		}
	}
	
	if ((ret = socInit(SOC_buf, SOC_BUF_SIZE)) != 0){
		printf("socInit: 0x%08x\n", (unsigned int)ret);
		while (aptMainLoop()){
			gspWaitForVBlank();
			hidScanInput();
			u32 kDown = hidKeysDown();
			if (kDown & KEY_B) {
				socExitWrapper();
				gfxExit();
				exit(1);
			}
		}
	}
	
	printf("Socket API Initialized\n");
	
	// atexit() runs things in reverse order so this will be run before gfxExit()
	atexit(socExitWrapper);
	
	nntpcon con = nntpinit("nntp.aioe.org", 119);
	if (con.err != NNTPERR_OK){
		printf("Connection Error\nError Code: %d\n", con.err);
	}
	
	while (aptMainLoop()){
		hidScanInput();
		u32 kDown = hidKeysDown();
		
		if (kDown & KEY_START) break;
		if (kDown & KEY_A) {
			printf("Finding available groups\n");
			nntpgroups groups = nntp_get_groups(con);
			if (groups.err != NNTPERR_OK){
				printf("Error: %d\n", groups.err);
				if (groups.err == NNTPERR_READ) printf("Errno: %d\n", groups.errcode);
			}
			
			printf("Found %ld groups\n", groups.len);
			
			sleep(1);
			if (groups.groups != NULL){
				for(u32 i = 0; i < groups.len; i++){
					if (groups.groups[i] != NULL) free(groups.groups[i]);
				}
				free(groups.groups);
			}
			
			printf("Done\n");
		}
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		gspWaitForVBlank();
	}
	close(con.socketfd);
	
	exit(0);
}