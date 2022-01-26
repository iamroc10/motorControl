/**
*	@file client.c
*	@brief This file represents the client, i.e. the engine that must receive its position and constantly send data to the other engine (server) to move when necessary. 
*	@author ROC BENAIGES MORAGREGA
*/

#include <taskLib.h>
#include <stdio.h>
#include <kernelLib.h>
#include <semLib.h>
#include <intLib.h>
#include <iv.h>
#include <xlnx_zynq7k.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inetLib.h>
#include <sockLib.h>

#include "ircs.h"


volatile int irc_a, irc_b;
int sockd;  							/**< Variable for creating a UDP socket */
struct sockaddr_in my_addr, srv_addr;	/**< Variable to configure the client adress */
char buf[32];							/**< Buffer of size 32 used to store and then send the data */
int motorPos;							/**< Position of the motor */


/**
*	This method creates the connection, creating a UDP socket, configuring the client adress and IP
*	@author ROC BENAIGES MORAGREGA
*/
void init_connection() 
{
	/* Create a UDP socket */
	sockd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockd == -1)
	{
		perror("Socket creation error");
		exit(1);
	}

	/* Configure client address */
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = 0;

	bind(sockd, (struct sockaddr*)&my_addr, sizeof(my_addr));

	srv_addr.sin_family = AF_INET;
	inet_aton("192.168.202.216", &srv_addr.sin_addr);
	srv_addr.sin_port = htons(5000);
}


/**
*	This method will be used to send the data (motor position) to the server adress.
*	@author ROC BENAIGES MORAGREGA
*/
void send_data () 
{
	int i;
	
	while (1) {
		/*printf("Motor position %d\n", motorPos);*/
		snprintf(buf, 32, "%d ", motorPos);
		/*printf("Send %s => ", buf);*/
		i = sendto(sockd, &motorPos, 32, 0, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
		/*printf("size = %d\n", i);*/
		taskDelay(1);
	}
}


/**
	*This method will be used to constantly obtain the position of the engine to check if it reads it correctly.
*	@author ROC BENAIGES MORAGREGA
*/
void get_position () 
{
        while(1)
                printf("motorPos= %d \n ", motorPos);
}


/**
	*This method will be used as a main and will call all the functions and manage the motor of the "client".
	*	@author ROC BENAIGES MORAGREGA
*/	
void motorClient(void)
{
        TASK_ID sendData, motorGetPosTask; /**< ID of the tasks sendData and motorGetPosTask created by taskSpawn() */
        motorPos = 0;

        irc_init();
        init_connection();
        /*motorGetPosTask = taskSpawn("motorGetPos", 50, 0, 4096, (FUNCPTR) get_position, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);*/
        sendData = taskSpawn("sendData", 50, 0, 4096, (FUNCPTR) send_data, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

        /*while(1);
        
        taskDelay(10000);*/
        getchar();
        printf("Out of play time.\n");

        irc_disable();
        taskDelete(sendData);
        taskDelete(motorGetPosTask);
}
