#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "SocketCAN.h"

volatile sig_atomic_t running = 1;

static void interrupt_handler(int signum){
	running = 0;
}

int main(void){

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &interrupt_handler;
	sigaction(SIGINT, &sa, NULL);


	SocketCAN_T_ptr cansock;
	SocketCAN_Config_T cfg = {
		.iface_number = IFACE_CAN0,
		.bitrate = BITRATE_500000KBS
	};

	cansock = SocketCAN_Create(&cfg);

	if(cansock){
		printf("Successfully created cansock\r\n");
	}
	else{
		fprintf(stderr, "Unable to create cansock\r\n");
		exit(EXIT_FAILURE);
	}

	while(running){
		
		SocketCAN_ReadMessage(cansock);

	}

	printf("Exiting application\r\n");
	SocketCAN_Destroy(cansock);
	
	return 0;
}
